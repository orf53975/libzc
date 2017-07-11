/*
 *  zc - zip crack library
 *  Copyright (C) 2012-2017 Marc Ferland
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * References:
 * http://en.wikipedia.org/wiki/Modular_multiplicative_inverse
 * http://ca.wiley.com/WileyCDA/WileyTitle/productCd-047011486X.html
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "libzc.h"
#include "libzc_private.h"
#include "crc32.h"
#include "key2_reduce.h"

#define k2(index) w->key2_final[index]
#define k1(index) w->key1_final[index]
#define k0(index) w->key0_final[index]
#define cipher(index) ptext->ciphertext[index]
#define plaintext(index) ptext->plaintext[index]
#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)

struct zc_crk_ptext {
    struct zc_ctx *ctx;
    int refcount;
    const uint8_t *plaintext;
    const uint8_t *ciphertext;
    size_t size;
    struct ka *key2;
    struct key2r *k2r;
    uint8_t lsbk0_lookup[256][2];
    uint32_t lsbk0_count[256];
};

struct worker {
    uint32_t key2_final[13];
    uint32_t key1_final[13];
    uint32_t key0_final[13];
    size_t offset;
    size_t amount;
    bool found;
    struct zc_key inter_rep;     /* intermediate representation of the key */
    const struct zc_crk_ptext *ptext;
    pthread_t thread;
};

static uint8_t generate_key3(const struct zc_crk_ptext *ptext, uint32_t i)
{
    return (plaintext(i) ^ cipher(i));
}

static void generate_key0lsb(struct zc_crk_ptext *ptext)
{
    /* reset lsb counters to 0 */
    memset(ptext->lsbk0_count, 0, 256 * sizeof(uint32_t));

    for (uint32_t i = 0, p = 0; i < 256; ++i, p += MULTINV) {
        uint8_t msbp = msb(p);
        ptext->lsbk0_lookup[msbp][ptext->lsbk0_count[msbp]++] = i;
    }
}

ZC_EXPORT struct zc_crk_ptext *zc_crk_ptext_ref(struct zc_crk_ptext *ptext)
{
    if (!ptext)
        return NULL;
    ptext->refcount++;
    return ptext;
}

ZC_EXPORT struct zc_crk_ptext *zc_crk_ptext_unref(struct zc_crk_ptext *ptext)
{
    if (!ptext)
        return NULL;
    ptext->refcount--;
    if (ptext->refcount > 0)
        return ptext;
    dbg(ptext->ctx, "ptext %p released\n", ptext);
    ka_free(ptext->key2);
    key2r_free(ptext->k2r);
    free(ptext);
    return NULL;
}

ZC_EXPORT int zc_crk_ptext_new(struct zc_ctx *ctx, struct zc_crk_ptext **ptext)
{
    struct zc_crk_ptext *new;

    new = calloc(1, sizeof(struct zc_crk_ptext));
    if (!new)
        return -1;

    if (key2r_new(&new->k2r)) {
        free(new);
        return -1;
    }

    generate_key0lsb(new);
    new->ctx = ctx;
    new->refcount = 1;
    new->key_found = false;
    *ptext = new;

    dbg(ctx, "ptext %p created\n", new);

    return 0;
}

ZC_EXPORT int zc_crk_ptext_set_text(struct zc_crk_ptext *ptext,
                                    const uint8_t *plaintext,
                                    const uint8_t *ciphertext,
                                    size_t size)
{
    if (size < 13)
        return -1;

    ptext->plaintext = plaintext;
    ptext->ciphertext = ciphertext;
    ptext->size = size;

    return 0;
}

ZC_EXPORT int zc_crk_ptext_key2_reduction(struct zc_crk_ptext *ptext)
{
    struct ka *key2i_plus_1;
    struct ka *key2i;
    uint8_t key3i;
    uint8_t key3im1;

    /* first gen key2 */
    key3i = generate_key3(ptext, ptext->size - 1);
    key2i_plus_1 = key2r_compute_first_gen(key2r_get_bits_15_2(ptext->k2r, key3i));
    if (!key2i_plus_1)
        return -1;

    /* allocate space for second array */
    if (ka_alloc(&key2i, pow2(22))) {
        ka_free(key2i_plus_1);
        return -1;
    }

    /* perform reduction */
    const uint32_t start_index = ptext->size - 2;
    for (uint32_t i = start_index; i >= 12; --i) {
        key3i = generate_key3(ptext, i);
        key3im1 = generate_key3(ptext, i - 1);
        key2r_compute_next_array(key2i_plus_1,
                                 key2i,
                                 key2r_get_bits_15_2(ptext->k2r, key3i),
                                 key2r_get_bits_15_2(ptext->k2r, key3im1),
                                 i == start_index ? KEY2_MASK_6BITS : KEY2_MASK_8BITS);

        ka_uniq(key2i);
        SWAP(key2i, key2i_plus_1);
    }

    ka_squeeze(key2i_plus_1); /* note: we swapped key2i and key2i+1 */

    ptext->key2 = key2i_plus_1;  /* here, key2i_plus_1, is the array at
                                 * index 13 (n=14) this leaves 13
                                 * bytes for the actual attack */
    ka_free(key2i);
    return 0;
}

static void ptext_final_deinit(struct ka **key2)
{
    for (uint32_t i = 0; i < 12; ++i) {
        if (key2[i]) {
            ka_free(key2[i]);
            key2[i] = NULL;
        }
    }
}

static int ptext_final_init(struct ka **key2)
{
    for (uint32_t i = 0; i < 12; ++i) {
        if (ka_alloc(&key2[i], 64)) { /* FIXME: 64 ? */
            ptext_final_deinit(key2);
            return -1;
        }
    }
    return 0;
}

static uint32_t compute_key1_msb(const struct zc_crk_ptext *ptext, uint32_t current_idx)
{
    const uint32_t key2i = k2(current_idx);
    const uint32_t key2im1 = k2(current_idx - 1);
    return (key2i << 8) ^ crc_32_invtab[key2i >> 24] ^ key2im1;
}

static bool verify_key0(struct zc_crk_ptext *ptext, uint32_t key0,
                        uint32_t start, uint32_t stop)
{
    for (uint32_t i = start; i < stop; ++i) {
        key0 = crc32(key0, plaintext(i));
        if (mask_lsb(key0) != k0(i + 1))
            return false;
    }
    return true;
}

static void compute_one_intermediate_int_rep(uint8_t cipher, uint8_t *plaintext, struct zc_key *k)
{
    k->key2 = crc32inv(k->key2, msb(k->key1));
    k->key1 = ((k->key1 - 1) * MULTINV) - lsb(k->key0);
    uint32_t tmp = k->key2 | 3;
    uint32_t key3 = lsb((tmp * (tmp ^ 1)) >> 8);
    *plaintext = cipher ^ key3;
    k->key0 = crc32inv(k->key0, *plaintext);
}

static int compute_intermediate_internal_rep(struct zc_crk_ptext *ptext, struct zc_key *k)
{
    uint32_t i = 4;

    k->key2 = k2(i);
    k->key1 = k1(i);
    /* key0 is already set */

    do {
        uint8_t p;
        compute_one_intermediate_int_rep(cipher(i - 1), &p, k);
        if (p != plaintext(i - 1))
            break;
        --i;
    } while (i > 0);

    if (i == 0) {
        ptext->inter_rep = *k;
        return 0;
    }
    return -1;
}

static void compute_key0(struct zc_crk_ptext *ptext)
{
   struct zc_key k = { .key0 = 0x0, .key1 = 0x0, .key2 = 0x0 };

    /* calculate key0_6{0..15} */
    k.key0 = (k0(7) ^ crc_32_tab[k0(6) ^ plaintext(6)]) << 8;
    k.key0 = (k.key0 | k0(6)) & 0x0000ffff;

    /* calculate key0_5{0..23} */
    k.key0 = (k.key0 ^ crc_32_tab[k0(5) ^ plaintext(5)]) << 8;
    k.key0 = (k.key0 | k0(5)) & 0x00ffffff;

    /* calculate key0_4{0..31} */
    k.key0 = (k.key0 ^ crc_32_tab[k0(4) ^ plaintext(4)]) << 8;
    k.key0 = (k.key0 | k0(4));

    /* verify against known bytes */
    if (!verify_key0(ptext, k.key0, 4, 12))
        return;

    if (compute_intermediate_internal_rep(ptext, &k) == 0)
        ptext->key_found = true;
}

static void recurse_key1(struct worker *w, uint32_t current_idx)
{
    if (current_idx == 3) {
        compute_key0(w);
        return;
    }

    uint32_t key1i = k1(current_idx);
    uint32_t rhs_step1 = (key1i - 1) * MULTINV;
    uint32_t rhs_step2 = (rhs_step1 - 1) * MULTINV;
    uint8_t diff = msb(rhs_step2 - (mask_msb(k1(current_idx - 2))));

    for (uint32_t c = 2; c != 0; --c, --diff) {
        for (uint32_t i = 0; i < w->ptext->lsbk0_count[diff]; ++i) {
            uint32_t lsbkey0i = w->ptext->lsbk0_lookup[diff][i];
            if (mask_msb(rhs_step1 - lsbkey0i) == mask_msb(k1(current_idx - 1))) {
                w->key1_final[current_idx - 1] = rhs_step1 - lsbkey0i;
                w->key0_final[current_idx] = lsbkey0i;
                recurse_key1(w, current_idx - 1);
            }
        }
    }
}

static void compute_key1(struct worker *w)
{
    /* find matching msb, section 3.3 from Biham & Kocher */
    for (uint32_t i = 0; i < pow2(24); ++i) {
        const uint32_t key1_12_tmp = mask_msb(k1(12)) | i;
        const uint32_t key1_11_tmp = (key1_12_tmp - 1) * MULTINV;
        if (mask_msb(key1_11_tmp) == mask_msb(k1(11))) {
            w->key1_final[12] = key1_12_tmp;
            recurse_key1(ptext, 12);
        }
    }
}

static void recurse_key2(struct worker *w, struct ka **array, uint32_t current_idx)
{
    uint8_t key3im1;
    uint8_t key3im2;

    if (current_idx == 1) {
        compute_key1(w);
        return;
    }

    key3im1 = generate_key3(w->ptext, current_idx - 1);
    key3im2 = generate_key3(w->ptext, current_idx - 2);

    /* empty array before appending new keys */
    ka_empty(array[current_idx - 1]);

    key2r_compute_single(k2(current_idx),
                         array[current_idx - 1],
                         key2r_get_bits_15_2(w->ptext->k2r, key3im1),
                         key2r_get_bits_15_2(w->ptext->k2r, key3im2),
                         KEY2_MASK_8BITS);

    ka_uniq(array[current_idx - 1]);

    for (uint32_t i = 0; i < array[current_idx - 1]->size; ++i) {
        w->key2_final[current_idx - 1] = ka_at(array[current_idx - 1], i);
        w->key1_final[current_idx] = compute_key1_msb(w->ptext, current_idx) << 24;
        recurse_key2(w, array, current_idx - 1);
    }
}

ZC_EXPORT size_t zc_crk_ptext_key2_count(const struct zc_crk_ptext *ptext)
{
    if (ptext->key2)
        return ptext->key2->size;
    return 0;
}

ZC_EXPORT int zc_crk_ptext_attack(struct zc_crk_ptext *ptext, struct zc_key *out_key)
{
    struct ka *array[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (ptext_final_init(array))
        return -1;

    ptext->key_found = false;
    for (uint32_t i = 0; i < ptext->key2->size; ++i) {
        ptext->key2_final[12] = ptext->key2->array[i];
        recurse_key2(ptext, array, 12);
        if (ptext->key_found) {
            out_key->key0 = ptext->inter_rep.key0;
            out_key->key1 = ptext->inter_rep.key1;
            out_key->key2 = ptext->inter_rep.key2;
            break;
        }
    }

    ptext_final_deinit(array);
    return (ptext->key_found == true ? 0 : -1);
}

#define THREADS 1

ZC_EXPORT int zc_crk_ptext_attack(struct zc_crk_ptext *ptext, struct *out_key)
{
    struct worker *w;

    w = calloc(THREADS, sizeof(struct worker));
    if (!w)
        return -1;

    const size_t quot = ptext->key2->size / THREADS;
    const size_t rem  = ptext->key2->size % THREADS;

    for (int i = 0; i < THREADS; ++i) {
        w->offset = i * quot;
        w->amount = i == THREADS - 1 ? quot + rem : quot;
        printf("thread: %zu - %zu\n", w->offset, w->amount);
        pthread_create(&w[i].thread, NULL, worker, &w[i]);
    }

    /* TODO: create list of threads */
    for (int i = 0; i < THREADS; ++i) {
        pthread_join(w[i].thread, NULL);
        if (w[i].found) {
            out_key->key0 = w[i].inter_rep.key0;
            out_key->key1 = w[i].inter_rep.key1;
            out_key->key2 = w[i].inter_rep.key2;
        }
    }

    free(w);

    return 0;
}

void * worker(void *p)
{
    struct worker *w = (struct worker *)p;

    struct ka *array[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    if (ptext_final_init(array))
        return -1;

    for (size_t i = 0; i < w->amount; ++i) {
        w->key2_final[12] = w->ptext->key2->array[w->offset + i];
        recurse_key2(w, array, 12);
        if (w->found)
            break;                /* parent thread will collect result */
    }

    ptext_final_deinit(array);
    return NULL;
}

ZC_EXPORT int zc_crk_ptext_find_internal_rep(const struct zc_key *start_key,
                                             const uint8_t *ciphertext, size_t size,
                                             struct zc_key *internal_rep)
{
    struct zc_key k;
    uint32_t i;

    /* the cipher text also includes the 12 prepended bytes */
    if (size < 12)
        return -1;

    i = size - 1;
    k = *start_key;
    do {
        uint8_t p;
        compute_one_intermediate_int_rep(ciphertext[i], &p, &k);
    } while (i--);

    *internal_rep = k;
    return 0;
}

ZC_EXPORT int zc_crk_ptext_find_password(struct zc_crk_ptext *ptext,
                                         const struct zc_key *internal_rep,
                                         char *out, size_t len)
{
    return find_password(ptext->lsbk0_lookup,
                         ptext->lsbk0_count,
                         internal_rep,
                         out,
                         len);
}

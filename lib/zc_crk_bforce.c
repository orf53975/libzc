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

#include <pthread.h>
#ifdef __APPLE__
#include "pthread_barrier.h"
#endif
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "list.h"
#include "libzc.h"
#include "pwstream.h"
#include "libzc_private.h"

#define LEN 64

/* bruteforce cracker */
struct zc_crk_bforce {
    struct zc_ctx *ctx;
    int refcount;

    /* validation data */
    struct validation_data vdata[VDATA_MAX];
    size_t vdata_size;
    unsigned char *cipher;
    size_t cipher_size;
    bool cipher_is_deflated;
    uint32_t original_crc;

    /* zip filename */
    char *filename;

    /* initial password */
    char ipw[ZC_PW_MAXLEN + 1];
    size_t ipwlen;
    size_t maxlen;

    /* character set */
    char set[ZC_CHARSET_MAXLEN + 1];
    size_t setlen;

    /* password streams */
    struct pwstream **pws;
    size_t pwslen;

    struct list_head workers_head;
    struct list_head cleanup_head;
    pthread_barrier_t barrier;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    bool found;
};

struct worker {
    struct list_head workers;
    struct list_head cleanup;
    pthread_t thread_id;
    size_t id;
    char pw[ZC_PW_MAXLEN + 1];
    bool found;
    unsigned char *inflate;
    unsigned char *plaintext;
    struct zlib_state *zlib;

    struct hash {
        uint8_t check[LEN];
        uint32_t initk0[LEN];
        uint32_t initk1[LEN];
        uint32_t initk2[LEN];
        uint32_t k0[LEN];
        uint32_t k1[LEN];
        uint32_t k2[LEN];
    } h;

    struct zc_crk_bforce *crk;
};

static size_t unique(char *str, size_t len)
{
    if (len <= 1)
        return len;

    size_t j = 0;
    for (size_t i = 1; i < len; ++i) {
        if (str[i] != str[j])
            str[++j] = str[i];
    }

    return j + 1;
}

static int compare_char(const void *a, const void *b)
{
    return (*(const char *)a - * (const char *)b);
}

static size_t sanitize_set(char *set, size_t len)
{
    qsort(set, len, sizeof(char), compare_char);
    size_t newlen = unique(set, len);
    set[newlen] = '\0';
    return newlen;
}

static bool pw_in_set(const char *pw, const char *set, size_t len)
{
    for (size_t i = 0; pw[i] != '\0'; ++i) {
        if (!memchr(set, pw[i], len))
            return false;
    }
    return true;
}

static bool test_password(struct worker *w, const struct zc_key *key)
{
    int err;

    decrypt(w->crk->cipher, w->plaintext, w->crk->cipher_size, key);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (w->crk->cipher_is_deflated)
        err = inflate_buffer(w->zlib,
			     &w->plaintext[12],
                             w->crk->cipher_size - 12,
                             w->inflate,
                             INFLATE_CHUNK,
                             w->crk->original_crc);
    else
        err = test_buffer_crc(&w->plaintext[12],
                              w->crk->cipher_size - 12,
                              w->crk->original_crc);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    return err ? false : true;
}

static bool try_decrypt(const struct zc_crk_bforce *crk, const struct zc_key *base)
{
    struct zc_key key;
    for (size_t i = 0; i < crk->vdata_size; ++i) {
        reset_encryption_keys(base, &key);
        if (decrypt_header(crk->vdata[i].encryption_header, &key, crk->vdata[i].magic))
            return false;
    }
    return true;
}

static void do_work_recurse(struct worker *w, size_t level,
                            size_t level_count, char *pw, struct zc_key *cache,
                            struct entry *limit)
{
    const struct zc_crk_bforce *crk = w->crk;
    int first = limit[0].initial;
    int last = limit[0].stop + 1;

    if (level == 1) {
        for (int p = first; p < last; ++p) {
            update_keys(crk->set[p], &cache[level_count - 1], &cache[level_count]);
            if (try_decrypt(crk, &cache[level_count])) {
                if (test_password(w, &cache[level_count])) {
		    pw[level_count - 1] = crk->set[p];
                    w->found = true;
                    pthread_exit(w);
		}
            }
        }
    } else {
        size_t i = level_count - level;
        for (int p = first; p < last; ++p) {
            pw[i] = crk->set[p];
            update_keys(pw[i], &cache[i], &cache[i + 1]);
            do_work_recurse(w, level - 1, level_count, pw, cache, &limit[1]);
        }
    }
    limit[0].initial = limit[0].start;
}

static int try_decrypt_fast(const struct zc_crk_bforce *crk, struct hash *h)
{
    int ret = 0;
    uint8_t *c = h->check;

    /* first pass */
    for (int i = 0; i < 11; ++i) {
        uint8_t header = crk->vdata[0].encryption_header[i];
        for (int j = 0; j < LEN; ++j)
            c[j] = header ^ decrypt_byte(h->k2[j]);

        /* update key0 */
        for (int j = 0; j < LEN; ++j)
            h->k0[j] = crc32(h->k0[j], c[j]);

        /* update key1 */
        for (int j = 0; j < LEN; ++j)
            h->k1[j] = (h->k1[j] + (h->k0[j] & 0xff)) * MULT + 1;

        /* update key2 */
        for (int j = 0; j < LEN; ++j)
            h->k2[j] = crc32(h->k2[j], h->k1[j] >> 24);
    }

    uint8_t header = crk->vdata[0].encryption_header[11];
    uint8_t magic = crk->vdata[0].magic;
    for (int j = 0; j < LEN; ++j) {
        c[j] = header ^ decrypt_byte(h->k2[j]) ^ magic;
	ret |= c[j];
    }
    return ret;
}

static int try_decrypt2(const struct zc_crk_bforce *crk, struct worker *w, int len)
{
    struct zc_key key;
    struct hash *h = &w->h;

    for (int i = 0; i < len; ++i) {
	if (h->check[i])
	    continue;
        key.key0 = h->initk0[i];
        key.key1 = h->initk1[i];
        key.key2 = h->initk2[i];
        for (size_t j = 1; j < crk->vdata_size; ++j) {
            if (decrypt_header(crk->vdata[j].encryption_header, &key, crk->vdata[j].magic))
                continue;
        }
        key.key0 = h->initk0[i];
        key.key1 = h->initk1[i];
        key.key2 = h->initk2[i];
        if (test_password(w, &key))
            return i;
    }

    return -1;
}

/*
    out[5] = c % in[5];
    out[4] = (c / in[5]) % in[4];
    out[3] = (c / in[5] / in[4]) % in[3];
    out[2] = (c / in[5] / in[4] / in[3]) % in[2];
    out[1] = (c / in[5] / in[4] / in[3] / in[2]) % in[1];
    out[0] = (c / in[5] / in[4] / in[3] / in[2] / in[1]) % in[0];
 */
static void indexes_from_raw_counter(uint64_t c, const int *in, int* out)
{
    int i = 5;
    out[i] = c;
    for (; i >= 0; --i)
        out[i - 1] = out[i] / in[i];
    for (i = 0; i < 6; ++i)
        out[i] %= in[i];
}

static void do_work_recurse2(struct worker *w, size_t level,
                             size_t level_count, char *pw, struct zc_key *cache,
                             struct entry *limit)
{
    const struct zc_crk_bforce *crk = w->crk;
    if (level_count > 5 && level == 6) {
        int first[6], last[6], p[6], out[6], in[6], ret;
        uint64_t pwi = 0;

        for (int i = 0; i < 6; ++i) {
            first[i] = limit[i].initial;
            last[i] = limit[i].stop + 1;
        }

        for (p[0] = first[0]; p[0] < last[0]; ++p[0]) {
            update_keys(crk->set[p[0]], &cache[0], &cache[1]);
            for (p[1] = first[1]; p[1] < last[1]; ++p[1]) {
                update_keys(crk->set[p[1]], &cache[1], &cache[2]);
                for (p[2] = first[2]; p[2] < last[2]; ++p[2]) {
                    update_keys(crk->set[p[2]], &cache[2], &cache[3]);
                    for (p[3] = first[3]; p[3] < last[3]; ++p[3]) {
                        update_keys(crk->set[p[3]], &cache[3], &cache[4]);
                        for (p[4] = first[4]; p[4] < last[4]; ++p[4]) {
                            update_keys(crk->set[p[4]], &cache[4], &cache[5]);
                            for (p[5] = first[5]; p[5] < last[5]; ++p[5]) {
                                update_keys(crk->set[p[5]], &cache[5], &cache[6]);

                                /* save password hashes */
                                w->h.initk0[pwi % LEN] = w->h.k0[pwi % LEN] = cache[6].key0;
                                w->h.initk1[pwi % LEN] = w->h.k1[pwi % LEN] = cache[6].key1;
                                w->h.initk2[pwi % LEN] = w->h.k2[pwi % LEN] = cache[6].key2;

                                if (++pwi % LEN)
                                    continue;

                                if (!try_decrypt_fast(crk, &w->h))
                                    continue;

                                ret = try_decrypt2(crk, w, LEN);
                                if (ret < 0)
                                    continue;

                                /* copy the loop length to 'in' */
                                for (int i = 0; i < 6; ++i)
                                    in[i] = last[i] - first[i];

                                /* adjust the counter to the index of
                                 * the correct hash */
                                pwi = pwi - (LEN - 1 - ret) - 1;
                                indexes_from_raw_counter(pwi, in, out);
                                for (int i = 0; i < 6; ++i)
                                    pw[i] = crk->set[out[i] + first[i]];

                                w->found = true;
                                pthread_exit(w);
                            }
                        }
                    }
                }
            }
        }

	/* fill the check bytes with zeros so try_decrypt2 will test
	 * all the remaining hashes */
	memset(w->h.check, 0, LEN);

        ret = try_decrypt2(crk, w, pwi % LEN);
        if (ret < 0)
            return;

        for (int i = 0; i < 6; ++i)
            in[i] = last[i] - first[i];

        pwi = pwi - ((pwi % LEN) - 1 - ret) - 1;
        indexes_from_raw_counter(pwi, in, out);
        for (int i = 0; i < 6; ++i)
            pw[i] = crk->set[out[i] + first[i]];

        w->found = true;
        pthread_exit(w);
    } else {
        int first = limit[0].initial;
        int last = limit[0].stop + 1;
        for (int p = first; p < last; ++p) {
            pw[0] = crk->set[p];
            update_keys(pw[0], &cache[0], &cache[1]);
            do_work_recurse2(w, level - 1, level_count, &pw[1], &cache[1], &limit[1]);
        }
    }
    limit[0].initial = limit[0].start;
}

static void fill_limits(struct pwstream *pws, struct entry *limit, size_t count,
                        size_t stream)
{
    for (size_t i = 0, j = count - 1; i < count; ++i, --j)
        limit[i] = *pwstream_get_entry(pws, stream, j);
}

static void do_work(struct worker *w, struct pwstream *pws,
                    size_t stream, char *pw)
{
    size_t level = pwstream_get_pwlen(pws);
    struct zc_key cache[level + 1];
    struct entry limit[level];

    fill_limits(pws, limit, level, stream);
    memset(cache, 0, sizeof(struct zc_key) * (level + 1));
    set_default_encryption_keys(cache);

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    if (level < 6)
        do_work_recurse(w, level, level, pw, cache, limit);
    else
        do_work_recurse2(w, level, level, pw, cache, limit);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
}

static void worker_cleanup_handler(void *p)
{
    struct worker *w = (struct worker *)p;
    pthread_mutex_lock(&w->crk->mutex);
    list_del(&w->workers);
    list_add(&w->cleanup, &w->crk->cleanup_head);
    pthread_cond_signal(&w->crk->cond);
    pthread_mutex_unlock(&w->crk->mutex);
}

static void *worker(void *p)
{
    struct worker *w = (struct worker *)p;

    pthread_cleanup_push(worker_cleanup_handler, w);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    pthread_barrier_wait(&w->crk->barrier);

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    for (size_t i = 0; i < w->crk->pwslen; ++i) {
        if (pwstream_is_empty(w->crk->pws[i], w->id))
            continue;
        do_work(w, w->crk->pws[i], w->id, w->pw);
        pthread_testcancel();
    }

    pthread_cleanup_pop(1);
    return NULL;
}

static void dealloc_workers(struct zc_crk_bforce *crk)
{
    struct worker *w, *wtmp;
    list_for_each_entry_safe(w, wtmp,  &crk->workers_head, workers) {
        list_del(&w->workers);
        free(w->inflate);
        free(w->plaintext);
	inflate_destroy(w->zlib);
        free(w);
    }
}

static int alloc_workers(struct zc_crk_bforce *crk, size_t workers)
{
    for (size_t i = 0; i < workers; ++i) {
        struct worker *w = calloc(1, sizeof(struct worker));
        if (!w) {
            dealloc_workers(crk);
            return -1;
        }

        w->found = false;
        w->crk = crk;
        w->id = i;
        w->inflate = malloc(INFLATE_CHUNK);
        if (!w->inflate) {
            free(w);
            dealloc_workers(crk);
            return -1;
        }
        w->plaintext = malloc(crk->cipher_size);
        if (!w->plaintext) {
            free(w->inflate);
            free(w);
            dealloc_workers(crk);
            return -1;
        }
	if (inflate_new(&w->zlib) < 0) {
	    free(w->plaintext);
            free(w->inflate);
            free(w);
            dealloc_workers(crk);
            return -1;
	}
        list_add(&w->workers, &crk->workers_head);
    }

    return 0;
}

static void start_workers(struct zc_crk_bforce *crk)
{
    struct worker *w;

    pthread_mutex_lock(&crk->mutex);
    list_for_each_entry(w, &crk->workers_head, workers) {
       if (pthread_create(&w->thread_id, NULL, worker, w)) {
          perror("pthread_create failed");
          exit(1);
       }
    }
    pthread_mutex_unlock(&crk->mutex);
}

/* called while holding mutex */
static void cancel_workers(struct zc_crk_bforce *crk)
{
    struct worker *w;

    list_for_each_entry(w, &crk->workers_head, workers) {
       if (pthread_cancel(w->thread_id)) {
          perror("pthread_cancel failed");
          exit(1);
       }
    }
}

static void wait_workers(struct zc_crk_bforce *crk, size_t workers, char *pw, size_t len)
{
    int workers_left = workers;

    /* waits for workers on the 'cleanup' list */
    while (workers_left) {
        pthread_mutex_lock(&crk->mutex);
        while (list_empty(&crk->cleanup_head))
            pthread_cond_wait(&crk->cond, &crk->mutex);
        struct worker *w, *tempw;
        list_for_each_entry_safe(w, tempw, &crk->cleanup_head, cleanup) {
            list_del(&w->cleanup);
            pthread_join(w->thread_id, NULL);
            if (w->found) {
                memset(pw, 0, len);
                strncpy(pw, w->pw, len);
                crk->found = true;
                cancel_workers(crk);
            }
            free(w->inflate);
            free(w->plaintext);
	    inflate_destroy(w->zlib);
            free(w);
            --workers_left;
        }
        pthread_mutex_unlock(&crk->mutex);
    }
}

static void dealloc_pwstreams(struct zc_crk_bforce *crk)
{
    for (size_t l = 0; l < crk->pwslen; ++l) {
        if (crk->pws[l])
            pwstream_free(crk->pws[l]);
    }
    free(crk->pws);
}

static void fill_initial_pwstream(size_t *initial, const char *ipw, size_t ipwlen,
                                  const char *set, size_t setlen)
{
    for (size_t i = ipwlen - 1, j = 0; j < ipwlen; --i, ++j)
        initial[j] = (const char *)memchr(set, ipw[i], setlen) - set;
}

/* when generating the first streams, take into account the
 * initial password provided */
static int alloc_first_pwstream(struct pwstream **pws, const char *ipw, size_t ipwlen,
                                const char *set, size_t setlen, size_t workers)
{
    struct pwstream *tmp;
    size_t initial[ipwlen];

    if (pwstream_new(&tmp))
        return -1;

    fill_initial_pwstream(initial, ipw, ipwlen, set, setlen);
    pwstream_generate(tmp, setlen, ipwlen, workers, initial);

    *pws = tmp;

    return 0;
}

static int alloc_pwstreams(struct zc_crk_bforce *crk, size_t workers)
{
    const char *ipw = crk->ipw;
    size_t ipwlen = crk->ipwlen;
    size_t maxlen = crk->maxlen;
    size_t to_alloc = maxlen - ipwlen + 1;
    const char *set = crk->set;
    size_t setlen = crk->setlen;

    crk->pws = calloc(1, sizeof(struct pwstream *) * to_alloc);
    if (!crk->pws)
        return -1;

    if (alloc_first_pwstream(&crk->pws[0], ipw, ipwlen, set, setlen, workers)) {
        free(crk->pws);
        return -1;
    }

    crk->pwslen = 1;
    for (size_t i = 1; i < to_alloc; ++i) {
        if (pwstream_new(&crk->pws[i])) {
            dealloc_pwstreams(crk);
            return -1;
        }
        crk->pwslen++;
        pwstream_generate(crk->pws[i], setlen, ipwlen + i, workers, NULL);
    }

    return 0;
}

static int set_pwcfg(struct zc_crk_bforce *crk, const struct zc_crk_pwcfg *cfg)
{
    /* basic sanity checks */
    if (cfg->setlen == 0 ||
        cfg->setlen > ZC_CHARSET_MAXLEN  ||
        cfg->maxlen == 0 ||
        cfg->maxlen > ZC_PW_MAXLEN)
        return -1;

    if (strnlen(cfg->set, ZC_CHARSET_MAXLEN) != cfg->setlen)
        return -1;

    memcpy(crk->ipw, cfg->initial, ZC_PW_MAXLEN + 1);
    memcpy(crk->set, cfg->set, ZC_CHARSET_MAXLEN + 1);
    crk->maxlen = cfg->maxlen;
    crk->setlen = sanitize_set(crk->set, cfg->setlen);
    crk->ipwlen = strnlen(crk->ipw, ZC_PW_MAXLEN);

    if (!crk->ipwlen) {
        /* no initial password supplied, use first set character */
        crk->ipw[0] = crk->set[0];
        crk->ipw[1] = '\0';
        crk->ipwlen = 1;
        return 0;
    }

    if (crk->ipwlen > crk->maxlen)
        return -1;

    if (!pw_in_set(crk->ipw, crk->set, crk->setlen))
        return -1;

    return 0;
}

ZC_EXPORT int zc_crk_bforce_init(struct zc_crk_bforce *crk,
                                 const char *filename,
                                 const struct zc_crk_pwcfg *cfg)
{
    int err;

    err = set_pwcfg(crk, cfg);
    if (err) {
        err(crk->ctx, "failed to set password configuration\n");
        return -1;
    }

    err = fill_vdata(crk->ctx, filename, crk->vdata, VDATA_MAX);
    if (err < 1) {
        err(crk->ctx, "failed to read validation data\n");
        return -1;
    }

    crk->vdata_size = err;

    if (crk->cipher) {
        free(crk->cipher);
        crk->cipher = NULL;
    }
    err = fill_test_cipher(crk->ctx,
                           filename,
                           &crk->cipher,
                           &crk->cipher_size,
                           &crk->original_crc,
                           &crk->cipher_is_deflated);
    if (err) {
        err(crk->ctx, "failed to read cipher data\n");
        return -1;
    }

    if (crk->filename)
        free(crk->filename);
    crk->filename = strdup(filename);

    return 0;
}

ZC_EXPORT int zc_crk_bforce_new(struct zc_ctx *ctx, struct zc_crk_bforce **crk)
{
    struct zc_crk_bforce *tmp;
    int err;

    tmp = calloc(1, sizeof(struct zc_crk_bforce));
    if (!tmp)
        return -1;

    err = pthread_mutex_init(&tmp->mutex, NULL);
    if (err) {
        err(ctx, "pthread_mutex_init() failed: %s\n", strerror(err));
        free(tmp);
        return -1;
    }

    err = pthread_cond_init(&tmp->cond, NULL);
    if (err) {
        err(ctx, "pthread_cond_init() failed: %s\n", strerror(err));
        pthread_mutex_destroy(&tmp->mutex);
        free(tmp);
        return -1;
    }

    tmp->ctx = ctx;
    tmp->refcount = 1;

    INIT_LIST_HEAD(&tmp->workers_head);
    INIT_LIST_HEAD(&tmp->cleanup_head);

    *crk = tmp;

    dbg(ctx, "cracker %p created\n", tmp);
    return 0;
}

ZC_EXPORT struct zc_crk_bforce *zc_crk_bforce_ref(struct zc_crk_bforce *crk)
{
    if (!crk)
        return NULL;
    crk->refcount++;
    return crk;
}

ZC_EXPORT struct zc_crk_bforce *zc_crk_bforce_unref(struct zc_crk_bforce *crk)
{
    if (!crk)
        return NULL;
    crk->refcount--;
    if (crk->refcount > 0)
        return crk;
    if (crk->filename)
        free(crk->filename);
    if (crk->cipher)
        free(crk->cipher);
    pthread_cond_destroy(&crk->cond);
    pthread_mutex_destroy(&crk->mutex);
    free(crk);
    return NULL;
}

ZC_EXPORT const char *zc_crk_bforce_sanitized_charset(const struct zc_crk_bforce *crk)
{
    return crk->set;
}

ZC_EXPORT int zc_crk_bforce_start(struct zc_crk_bforce *crk, size_t workers,
                                  char *pw, size_t len)
{
    if (!workers || !len)
        return -1;

    if (alloc_pwstreams(crk, workers)) {
        err(crk->ctx, "failed to allocate password streams\n");
        goto err1;
    }

    if (alloc_workers(crk, workers)) {
       err(crk->ctx, "failed to allocate workers\n");
       goto err2;
    }

    if (pthread_barrier_init(&crk->barrier, NULL, workers)) {
       err(crk->ctx, "pthread_barrier_init failed\n");
       goto err3;
    }

    crk->found = false;
    start_workers(crk);
    wait_workers(crk, workers, pw, len);

    pthread_barrier_destroy(&crk->barrier);
    dealloc_pwstreams(crk);

    return crk->found ? 0 : 1; /* return -1 on error, 1 if not found else 0. */

err3:
    dealloc_workers(crk);
err2:
    dealloc_pwstreams(crk);
err1:
    return -1;
}

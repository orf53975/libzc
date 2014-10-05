/*
 *  zc - zip crack library
 *  Copyright (C) 2013  Marc Ferland
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

#include "key_table.h"
#include "qsort.h"

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

static void uint_qsort(unsigned int *arr, unsigned int n)
{
#define uint_lt(a,b) ((*a)<(*b))
   QSORT(unsigned int, arr, n, uint_lt);
}

static void sort(struct key_table *table)
{
   uint_qsort(table->array, table->size);
}

int key_table_new(struct key_table **table, size_t initial_size)
{
   struct key_table *tmp;

   if (initial_size == 0)
      return EINVAL;

   tmp = calloc(1, sizeof(struct key_table));
   if (tmp == NULL)
      return ENOMEM;

   tmp->array = calloc(1, initial_size * sizeof(unsigned int));
   if (tmp->array == NULL)
   {
      free(tmp);
      return ENOMEM;
   }
   
   tmp->capacity = tmp->size = initial_size;
   *table = tmp;
   
   return 0;
}

void key_table_free(struct key_table *table)
{
   if (table == NULL)
      return;
   free(table->array);
   free(table);
}

void key_table_append(struct key_table *table, unsigned int key)
{
   if (table->size < table->capacity)
   {
      table->array[table->size] = key;
      ++table->size;
      return;
   }

   table->capacity += 1024;
   table->array = realloc(table->array, table->capacity * sizeof(unsigned int));
   if (table->array == NULL)
      abort();

   table->array[table->size] = key;
   ++table->size;
}

void key_table_uniq(struct key_table *table)
{
   size_t i = 0;
   size_t j;
   
   if (table->size <= 1)
      return;

   sort(table);

   /* reduce by removing duplicates */
   for (j = 1; j < table->size; ++j)
   {
      if (table->array[j] != table->array[i])
         table->array[++i] = table->array[j];
   }

   table->size = i + 1;
}

void key_table_squeeze(struct key_table *table)
{
   if (table->size == table->capacity)
      return;
   table->array = realloc(table->array, table->size * sizeof(unsigned int));
   table->capacity = table->size;
}

void key_table_empty(struct key_table *table)
{
   /* future append will restart at 0 */
   table->size = 0;
}

void key_table_print(struct key_table *table, FILE *stream)
{
   for (unsigned int i = 0; i < table->size; ++i)
      fprintf(stream, "0x%0x\n", table->array[i]);
}
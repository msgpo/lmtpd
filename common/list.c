/*
 * Common code for implementing a linked list
 * Copyright (c) 2009-2011, albinoloverats ~ Software Development
 * email: webmaster@albinoloverats.net
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#define __LIST__H__
#include "list.h"
#include "common.h"
#include "logging.h"

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

extern list_t *list_create(int (*fn)(const void *, const void *))
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, fn);
    if (fn == NULL)
        list_compare_function = list_generic_compare;
    else
        list_compare_function = fn;
    return NEW_LIST;
}

extern void list_delete2(list_t **l, bool f)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    if (!l || !*l || *l == NEW_LIST)
        return;
    list_t *x = list_find_first(*l);
    while (true)
    {
        list_t *y = x->next;
        if (f && x->object)
            free((void *)x->object);
        x->object = NULL;
        x->prev = NULL;
        x->next = NULL;
        free(x);
        if (!y)
            break;
        x = y;
    }
    *l = NULL;
    return;
}

extern void list_append(list_t **l, const void * const restrict o)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p, %p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l, o);
    if (!l || *l == NEW_LIST)
    {
        if (!(*l = calloc(1, sizeof( list_t ))))
            die("out of memory @ %s:%d:%s [%zu]", __FILE__, __LINE__, __PRETTY_FUNCTION__, sizeof( list_t ));
        (*l)->object = o;
        return;
    }
    list_t *x = list_find_last(*l);
    list_t *n = calloc(1, sizeof( list_t ));
    if (!n)
        die("out of memory @ %s:%d:%s [%zu]", __FILE__, __LINE__, __PRETTY_FUNCTION__, sizeof( list_t ));
    n->prev = x;
    n->object = o;
    x->next = n;
    return;
}

extern void *list_remove(list_t **l, const uint64_t i)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p, %" PRIu64 ")", __FILE__, __LINE__, __PRETTY_FUNCTION__, l, i);
    if (!l || *l == NEW_LIST)
        return NULL;
    const uint64_t s = list_size(*l);
    if (i >= s)
        return NULL;
    list_t *x = list_move_to(*l, i);
    if (x->prev)
        x->prev->next = x->next;
    else
        *l = (*l)->next;
    if (x->next)
        x->next->prev = x->prev;
    const void *r = x->object;
    x->prev = NULL;
    x->next = NULL;
    free(x);
    x = NULL;
    return (void *)r;
}

extern list_t *list_move_to(list_t *l, const uint64_t i)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p, %" PRIu64 ")", __FILE__, __LINE__, __PRETTY_FUNCTION__, l, i);
    if (!l || l == NEW_LIST)
        return NULL;
    if (i >= list_size(l))
        return NULL;
    list_t *x = list_find_first(l);
    for (uint64_t j = 0; j < i; j++)
        x = x->next;
    return x;
}

extern void *list_get(list_t *l, const uint64_t i)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p, %" PRIu64 ")", __FILE__, __LINE__, __PRETTY_FUNCTION__, l, i);
    if (!l || l == NEW_LIST)
        return NULL;
    return (void *)list_move_to(l, i)->object;
}

extern uint64_t list_size(list_t *l)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    if (!l || l == NEW_LIST)
        return 0;
    list_t *x = list_find_first(l);
    uint64_t s = 1;
    while (true)
    {
        if (!x->next)
            break;
        x = x->next;
        s++;
    }
    return s;
}

extern void list_join(list_t *l, list_t *m)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p, %p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l, m);
    if (!l || l == NEW_LIST || !m || m == NEW_LIST)
        return;
    list_t *x = list_find_last(l);
    list_t *y = list_find_first(m);
    x->next = y;
    y->prev = x;
    return;
}

extern list_t *list_split(list_t *l)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    if (!l || l == NEW_LIST)
        return NULL;
    list_t *m = list_move_to(l, list_size(l) / 2);
    m->prev->next = NULL;
    m->prev = NULL;
    return m;
}

extern list_t *list_sort(list_t **l)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    if (!l || *l == NEW_LIST)
        return NULL;
    const uint64_t s = list_size(*l);
    if (s <= 1)
        return *l;
    list_t *r = list_split(*l);
    return list_msort(list_sort(l), list_sort(&r));
}

extern list_t *list_shuffle(list_t **l)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    if (!l || *l == NEW_LIST)
        return NULL;
    const uint64_t s = list_size(*l);
    if (s <= 1)
        return *l;
    srand48(time(0));
    for (uint64_t i = 0; i < SHUFFLE_FACTOR * s; i++)
    {
        uint64_t r = ((uint64_t)lrand48() << 32 | (uint64_t)lrand48()) % s;
        void *x = list_get(*l, r);
        list_remove(l, r);
        list_append(l, x);
    }
    return *l;
}

static list_t *list_msort(list_t *l, list_t *r)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p, %p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l, r);
    int64_t x = list_size(l);
    int64_t y = list_size(r);
    list_t *a = list_create(list_compare_function);
    while (x && y)
    {
        l = list_find_first(l);
        r = list_find_first(r);
        if (list_compare_function(l->object, r->object) <= 0)
        {
            list_append(&a, l->object);
            list_remove(&l, 0);
            x--;
        }
        else
        {
            list_append(&a, r->object);
            list_remove(&r, 0);
            y--;
        }
    }
    if (x)
        list_join(a, l);
    else
        list_join(a, r);
    return a;
}

static list_t *list_find_first(list_t * const restrict l)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    list_t *x = l;
    while (true)
        if (!x->prev)
            break;
        else
            x = x->prev;
    return x;
}

static list_t *list_find_last(list_t * const restrict l)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    list_t *x = l;
    while (true)
        if (!x->next)
            break;
        else
            x = x->next;
    return x;
}

static int list_generic_compare(const void *a, const void *b)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p, %p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, a, b);
    log_message(LOG_DEBUG, "using generic comparison function");
    return ((compare_id_t *)a)->id - ((compare_id_t *)b)->id;
}

#ifdef DEBUGGING
extern void list_debug(list_t *l)
{
    log_message(LOG_EVERYTHING, "%s:%d:%s(%p)", __FILE__, __LINE__, __PRETTY_FUNCTION__, l);
    list_t *x = list_find_first(l);
    uint64_t i = 0;
    while (true)
    {
        log_message(LOG_DEBUG, "object %02" PRIu64 ": %16p << %16p >> %16p = %p [%" PRIu64 "]", i, x->prev, x, x->next, x->object, x->object ? ((compare_id_t *)x->object)->id : (uint64_t)-1);
        x = x->next;
        if (!x)
            break;
        i++;
    }
    return;
}
#endif

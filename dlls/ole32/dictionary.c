/* Simple dictionary implementation using a linked list.
 * FIXME: a skip list would be faster.
 *
 * Copyright 2005 Juan Lang
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <assert.h>
#include <stdarg.h>
#include "windef.h"
#include "winbase.h"
#include "dictionary.h"

struct dictionary_entry
{
    void *key;
    void *value;
    struct dictionary_entry *next;
};

struct dictionary
{
    comparefunc comp;
    destroyfunc destroy;
    void *extra;
    struct dictionary_entry *head;
};

struct dictionary *dictionary_create(comparefunc c, destroyfunc d, void *extra)
{
    struct dictionary *ret;

    if (!c)
        return NULL;
    ret = HeapAlloc(GetProcessHeap(), 0, sizeof(struct dictionary));
    if (ret)
    {
        ret->comp = c;
        ret->destroy = d;
        ret->extra = extra;
        ret->head = NULL;
    }
    return ret;
}

void dictionary_destroy(struct dictionary *d)
{
    if (d)
    {
        struct dictionary_entry *p;

        for (p = d->head; p; )
        {
            struct dictionary_entry *next = p->next;

            if (d->destroy)
                d->destroy(p->key, p->value, d->extra);
            HeapFree(GetProcessHeap(), 0, p);
            p = next;
        }
        HeapFree(GetProcessHeap(), 0, d);
    }
}

/* Returns the address of the pointer to the node containing k.  (It returns
 * the address of either h->head or the address of the next member of the
 * prior node.  It's useful when you want to delete.)
 * Assumes h and prev are not NULL.
 */
static struct dictionary_entry **dictionary_find_internal(struct dictionary *d,
 const void *k)
{
    struct dictionary_entry **ret = NULL;
    struct dictionary_entry *p;

    assert(d);
    /* special case for head containing the desired element */
    if (d->head && d->comp(k, d->head->key, d->extra) == 0)
        ret = &d->head;
    for (p = d->head; !ret && p && p->next; p = p->next)
    {
        if (d->comp(k, p->next->key, d->extra) == 0)
            ret = &p->next;
    }
    return ret;
}

void dictionary_insert(struct dictionary *d, const void *k, const void *v)
{
    struct dictionary_entry **prior;

    if (!d)
        return;
    if ((prior = dictionary_find_internal(d, k)))
    {
        if (d->destroy)
            d->destroy((*prior)->key, (*prior)->value, d->extra);
        (*prior)->key = (void *)k;
        (*prior)->value = (void *)v;
    }
    else
    {
        struct dictionary_entry *elem = (struct dictionary_entry *)
         HeapAlloc(GetProcessHeap(), 0, sizeof(struct dictionary_entry));

        if (!elem)
            return;
        elem->key = (void *)k;
        elem->value = (void *)v;
        elem->next = d->head;
        d->head = elem;
    }
}

BOOL dictionary_find(struct dictionary *d, const void *k, void **value)
{
    struct dictionary_entry **prior;
    BOOL ret = FALSE;

    if (!d)
        return FALSE;
    if (!value)
        return FALSE;
    if ((prior = dictionary_find_internal(d, k)))
    {
        *value = (*prior)->value;
        ret = TRUE;
    }
    return ret;
}

void dictionary_remove(struct dictionary *d, const void *k)
{
    struct dictionary_entry **prior, *temp;

    if (!d)
        return;
    if ((prior = dictionary_find_internal(d, k)))
    {
        temp = *prior;
        if (d->destroy)
            d->destroy((*prior)->key, (*prior)->value, d->extra);
        *prior = (*prior)->next;
        HeapFree(GetProcessHeap(), 0, temp);
    }
}

void dictionary_enumerate(struct dictionary *d, enumeratefunc e)
{
    struct dictionary_entry *p;

    if (!d)
        return;
    if (!e)
        return;
    for (p = d->head; p; p = p->next)
        e(p->key, p->value, d->extra);
}

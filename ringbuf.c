/* SPDX-License-Identifier: 0BSD */
/*
 * Copyright (C) 2023, by Scott Zuk <zooknotic@proton.me>
 */
/** 
 * @file ringbuf.c Simple array backed circular queue/ring buffer implementation
 * that can hold arbitrarily sized elements.
 */
#include <stdio.h>
#include <string.h>

#include "ringbuf.h"

#define RINGBUF_DEBUG 1

/**
 * Initialize a ringbuf struct.
 *
 * A pointer to the storage array is assumed to already exist
 * at buf that has enough space allocated to hold n_elems elements
 * of size elem_sz (in bytes). No memory allocation is performed,
 * so the queue cannot grow or shrink. The total buffer size must
 * be >= n_elems * elem_sz.
 * 
 * These semantics are similar to qsort() and bsearch().
 *
 * @param rb pointer to the ringbuf struct to initialize
 * @param buf pointer to the array buffer
 * @param n_elem maximum number of elements buf can hold
 * @param elem_sz the size (bytes) of each element
 * @return 0 on success, -1 if rb or buf are NULL
 */
int ringbuf_init(struct ringbuf *rb, const void *buf, size_t n_elem, size_t elem_sz)
{
    if (!rb || !buf) {
        return -1;
    }

    rb->buf = buf;
    rb->capacity = n_elem;
    rb->elem_sz = elem_sz;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->ops.elem_copy = NULL;
    rb->ops.elem_print = NULL;

    return 0;
}

/**
 * Checks if ringbuf is full.
 * @return 1 if ringbuf is full, 0 otherwise
 */
int ringbuf_full(const struct ringbuf *rb)
{
    return rb->count == rb->capacity;
}

/**
 * Checks if ringbuf is empty.
 * @return 1 if ringbuf is empty, 0 otherwise
 */
int ringbuf_empty(const struct ringbuf *rb)
{
    return 0 == rb->count;
}

#if RINGBUF_DEBUG
// print elems in order from head to tail
static void ringbuf_print_elems(const struct ringbuf *rb, void (*fn)(const void *))
{
    void *p;

    for (size_t i = 0; i < rb->count; i++) {
        p = (char *)rb->buf + (rb->elem_sz * ((rb->head + i) % rb->capacity));
        (*fn)(p);
        printf(" ");
    }
    printf("%s\n", rb->count == 0 ? "(empty)" : "");
}
#endif

/**
 * Adds an element to the tail of the ringbuf.
 * @param elem element to add. If NULL, the ringbuf is not modified and 0 is returned.
 * @return 0 on success, -1 if ringbuf is full
 */
int ringbuf_add_tail(struct ringbuf *rb, const void *elem)
{
    void *tp;

    if (!elem) {
        return 0;
    }
    if (ringbuf_full(rb)) {
        return -1;
    }

    tp = (char*)rb->buf + (rb->elem_sz * rb->tail);

    if (rb->ops.elem_copy) {
        (*rb->ops.elem_copy)(tp, elem);
    } else {
        memcpy(tp, elem, rb->elem_sz);
    }

    rb->tail = (rb->tail + 1) % rb->capacity;
    rb->count += 1;

#if RINGBUF_DEBUG
    // XXX: Demo only, rip it out if code used for anything real
    if (rb->ops.elem_print) {
        printf("%s: added elem: ", __FUNCTION__);
        (*rb->ops.elem_print)(elem);
        printf("\n");
        ringbuf_print_elems(rb, rb->ops.elem_print);
    }
#endif

    return 0;
}

/**
 * Removes an element from the head of the ringbuf.
 * @param elem where to copy the removed element. If NULL, the element is simply removed.
 * @return 0 on success, -1 if ringbuf is empty
 */
int ringbuf_remove_head(struct ringbuf *rb, void *elem)
{
    void *hp;

    if (ringbuf_empty(rb)) {
        return -1;
    }

    hp = (char *)rb->buf + (rb->elem_sz * rb->head);

    if (elem) {
        if (rb->ops.elem_copy) {
            (*rb->ops.elem_copy)(elem, hp);
        } else {
            memcpy(elem, hp, rb->elem_sz);
        }
#if RINGBUF_DEBUG
        // XXX:
        if (rb->ops.elem_print) {
            printf("%s: removed elem: ", __FUNCTION__);
            (*rb->ops.elem_print)(elem);
            printf("\n");
        }
#endif
    }
    // Not actually necessary, but can zero out the element just removed
    // from the buffer.
    memset(hp, 0, rb->elem_sz);

    rb->head = (rb->head + 1) % rb->capacity;
    rb->count -= 1;

#if RINGBUF_DEBUG
    // XXX: Demo only, rip it out if code used for anything real
    if (rb->ops.elem_print) {
        ringbuf_print_elems(rb, rb->ops.elem_print);
    }
#endif

    return 0;
}

/* SPDX-License-Identifier: 0BSD */
/*
 * Copyright (C) 2023, by Scott Zuk <zooknotic@proton.me>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#ifndef __RINGBUF_H__
#define __RINGBUF_H__

/** 
 * @file ringbuf.h Simple array backed circular queue/ring buffer implementation
 * that can hold arbitrarily sized elements.
 */

#include <stddef.h>

/**
 * The main ringbuf struct.
 */
struct ringbuf {
    /** Base pointer of element array */
    const void *buf;
    /** Maximum number of elements that can be stored */
    size_t capacity;  
    /** Size of each element (bytes) */
    size_t elem_sz;
    /** Head index */
    size_t head;
    /** Tail index */
    size_t tail;
    /** Number of currently stored elements */
    size_t count;

    /** Function pointers (callbacks) for custom operations */
    struct {
        /** 
         * Optional user provided function that implements copying an element from src
         * to dst. Called by ringbuf_add_tail/ringbuf_remove_head when adding/removing
         * elements to the ringbuf. If not set, memcpy is used by default.
         */ 
        void (*elem_copy)(void *dst, const void *src);
        /** 
         * Optional user provided function to implement printing an element.
         * If not set, elements are not printed.
         */
        void (*elem_print)(const void *elem);
    } ops;
};

int ringbuf_init(struct ringbuf *rb, const void *buf, size_t n_elem, size_t elem_sz);
int ringbuf_full(const struct ringbuf *rb);
int ringbuf_empty(const struct ringbuf *rb);
int ringbuf_add_tail(struct ringbuf *rb, const void *elem);
int ringbuf_remove_head(struct ringbuf *rb, void *elem);

#endif

/* SPDX-License-Identifier: 0BSD */
/*
 * Copyright (C) 2023, by Scott Zuk <zooknotic@proton.me>
 */

/**
 * @file ringbuf_test.c Example usage for a simple ringbuf implementation.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ringbuf.h"

#define ELEMS_BUF_LEN 8

#define queue_add(rb, el) ringbuf_add_tail((rb), (const void*)(el))
#define queue_remove(rb, el) ringbuf_remove_head((rb), (void*)(el))

struct my_struct {
    int id;
    char name[16];
};

static void ringbuf_print_stats(const struct ringbuf *rb)
{
    printf("ringbuf @ %p: buf=%p elem_sz=%zu head=%zu tail=%zu count=%zu capacity=%zu empty=%s full=%s\n",
            rb, rb->buf, rb->elem_sz, rb->head, rb->tail, rb->count, rb->capacity,
            ringbuf_empty(rb) ? "yes" : "no",
            ringbuf_full(rb) ? "yes" : "no");
}

// dump the underlying buffer
static void ringbuf_hex_dump(const struct ringbuf *rb)
{
    size_t buf_sz = rb->capacity * rb->elem_sz;
    const char *c = rb->buf;

    printf("%s (%zu): ", __FUNCTION__, buf_sz);
    for (; buf_sz > 0; buf_sz--, c++) {
        printf("%02x ", *c);
    }
    printf("\n");
}

static void elem_print_char(const void *elem)
{
    const char *c = (char *)elem;
    printf("%c", *c);
}

static void elem_print_int(const void *elem)
{
    const int *i = (int *)elem;
    printf("%d", *i);
}

static void elem_print_my_struct(const void *elem)
{
    const struct my_struct *s = (struct my_struct *)elem;
    printf("{%d, '%.*s'}", s->id, (int)sizeof(s->name), s->name);
}

static void copy_my_struct(void *dst, const void *src)
{
    const struct my_struct *src_elem = (struct my_struct *)src;
    struct my_struct *dst_elem = (struct my_struct *)dst;

    dst_elem->id = src_elem->id;
    snprintf(dst_elem->name, sizeof(dst_elem->name), "%s", src_elem->name);
}

static void test_queue_char(void)
{
    char buf[ELEMS_BUF_LEN];
    struct ringbuf rb;
    char my_elem;

    printf("==== %s START ====\n", __FUNCTION__);

    if (ringbuf_init(&rb, (void*)&buf, ELEMS_BUF_LEN, sizeof(buf[0])) < 0) {
        printf("ringbuf_init failed for char array\n");
        return;
    }

    rb.ops.elem_print = &elem_print_char;
    ringbuf_print_stats(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        my_elem = 'a' + i % 26;
        assert(0 == queue_add(&rb, &my_elem));
    }
    assert(-1 == queue_add(&rb, &my_elem)); // supposed to fail, queue is full

    assert(0 == queue_remove(&rb, &my_elem));
    my_elem = 'Z';
    assert(0 == queue_add(&rb, &my_elem)); // succeeds
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        assert(0 == queue_remove(&rb, &my_elem));
    }
    assert(-1 == queue_remove(&rb, &my_elem)); // supposed to fail, queue is empty
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    printf("==== %s END ====\n", __FUNCTION__);
}

static void test_queue_int(void)
{
    int buf[ELEMS_BUF_LEN];
    struct ringbuf rb;
    int my_elem;

    printf("==== %s START ====\n", __FUNCTION__);

    if (ringbuf_init(&rb, (void*)&buf, ELEMS_BUF_LEN, sizeof(buf[0])) < 0) {
        printf("ringbuf_init failed for int array\n");
        return;
    }

    rb.ops.elem_print = &elem_print_int;
    ringbuf_print_stats(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        my_elem = i + 1;
        assert(0 == queue_add(&rb, &my_elem));
    }
    assert(-1 == queue_add(&rb, &my_elem)); // supposed to fail, queue is full

    assert(0 == queue_remove(&rb, &my_elem));
    my_elem *= 10;
    assert(0 == queue_add(&rb, &my_elem)); // succeeds
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        assert(0 == queue_remove(&rb, &my_elem));
    }
    assert(-1 == queue_remove(&rb, &my_elem)); // supposed to fail, queue is empty
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    printf("==== %s END ====\n", __FUNCTION__);
}

static void test_queue_my_struct(void)
{
    struct my_struct buf[ELEMS_BUF_LEN];
    struct ringbuf rb;
    struct my_struct my_elem = {0};

    printf("==== %s START ====\n", __FUNCTION__);

    memset(buf, 0, sizeof(buf));

    if (ringbuf_init(&rb, (void*)&buf, ELEMS_BUF_LEN, sizeof(buf[0])) < 0) {
        printf("ringbuf_init failed for my_struct array\n");
        return;
    }

    rb.ops.elem_copy = &copy_my_struct;
    rb.ops.elem_print = &elem_print_my_struct;
    ringbuf_print_stats(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        my_elem.id = 100 + i;
        snprintf(my_elem.name, sizeof(my_elem.name), "name_%d", i);
        assert(0 == queue_add(&rb, &my_elem));
    }
    assert(-1 == queue_add(&rb, &my_elem)); // supposed to fail, queue is full

    assert(0 == queue_remove(&rb, &my_elem));
    memset(&my_elem, 0, sizeof(my_elem));
    assert(0 == queue_add(&rb, &my_elem)); // succeeds
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        assert(0 == queue_remove(&rb, &my_elem));
    }
    assert(-1 == queue_remove(&rb, &my_elem)); // supposed to fail, queue is empty
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    printf("==== %s END ====\n", __FUNCTION__);
}

// Uses a dynamically allocated backing array in cases where
// the number or size of the elements are too large to keep on
// the stack.
static void test_queue_my_struct_malloc(void)
{
    struct my_struct *buf;
    size_t buf_sz;
    struct ringbuf rb;
    struct my_struct my_elem = {0};

    printf("==== %s START ====\n", __FUNCTION__);

    buf_sz = ELEMS_BUF_LEN * sizeof(*buf);
    buf = malloc(buf_sz);

    if (!buf) {
        printf("failed to malloc buffer\n");
        return;
    } 
    memset(buf, 0, buf_sz);

    if (ringbuf_init(&rb, (void*)buf, ELEMS_BUF_LEN, sizeof(*buf)) < 0) {
        printf("ringbuf_init failed for my_struct array\n");
        free(buf);
        return;
    }

    rb.ops.elem_print = &elem_print_my_struct;
    ringbuf_print_stats(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        my_elem.id = 100 + i;
        snprintf(my_elem.name, sizeof(my_elem.name), "name_%d", i);
        assert(0 == queue_add(&rb, &my_elem));
    }
    assert(-1 == queue_add(&rb, &my_elem)); // supposed to fail, queue is full

    assert(0 == queue_remove(&rb, &my_elem));
    memset(&my_elem, 0, sizeof(my_elem));
    assert(0 == queue_add(&rb, &my_elem)); // succeeds
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    for (int i = 0; i < ELEMS_BUF_LEN; i++) {
        assert(0 == queue_remove(&rb, &my_elem));
    }
    assert(-1 == queue_remove(&rb, &my_elem)); // supposed to fail, queue is empty
    ringbuf_print_stats(&rb);
    ringbuf_hex_dump(&rb);

    free(buf);

    printf("==== %s END ====\n", __FUNCTION__);
}

int main(int argc, char** argv)
{
    test_queue_char();
    test_queue_int();
    test_queue_my_struct();
    test_queue_my_struct_malloc();

    return 0;
}

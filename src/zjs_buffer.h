// Copyright (c) 2016, Intel Corporation.

void zjs_buffer_init();

struct zjs_buffer_t {
    jerry_object_t *obj;
    uint8_t *buffer;
    uint32_t bufsize;
    struct zjs_buffer_t *next;
};

struct zjs_buffer_t *zjs_buffer_find(const jerry_object_t *obj);
jerry_object_t *zjs_buffer_create(uint32_t size);

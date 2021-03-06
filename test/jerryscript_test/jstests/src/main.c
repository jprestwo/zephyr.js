// Copyright (c) 2016, Intel Corporation.
// generated by jsrunner

#include <misc/printk.h>
#include <string.h>
#include <zephyr.h>

#include "jerry.h"

#include "script.h"

int main()
{
    printk("BEGIN JERRYSCRIPT\n");
    jerry_completion_code_t r = jerry_run_simple(script, strlen(script),JERRY_FLAG_EMPTY);
    if (r == JERRY_COMPLETION_CODE_OK) {
        printk("JERRYSCRIPT SUCCEEDED\n");
    } else {
        printk("JERRYSCRIPT FAILED\n");
    }
    jerry_cleanup();
    return 0;
}

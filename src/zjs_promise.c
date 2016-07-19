#include "zjs_util.h"
#include "zjs_promise.h"

struct promise {
    int32_t id;
    jerry_value_t then;         // Function registered from then()
    int32_t then_id;            // Callback ID for then JS callback
    jerry_value_t* then_args;   // Arguments for fulfilling promise
    uint32_t then_args_cnt;
    jerry_value_t catch;        // Function registered from catch()
    int32_t catch_id;           // Callback ID for catch JS callback
    jerry_value_t* catch_args;  // Arguments for rejecting promise
    uint32_t catch_args_cnt;
};

static uint64_t use = 0;

#define BIT_IS_SET(i, b)  ((i) & (1 << (b)))
#define BIT_SET(i, b)     ((i) |= (1 << (b)))
#define BIT_CLR(i, b)     ((i) &= ~(1 << (b)))

static int find_id(uint64_t* use)
{
    int id = 1;
    while (BIT_IS_SET(*use, id)) {
        ++id;
    }
    BIT_SET(*use, id);
    return id;
}

char* make_name(int32_t id, char* module)
{
    static char name[20];
    memcpy(name, module, strlen(module));
    name[strlen(module)] = '-';
    sprintf(name + strlen(module) + 1, "%u", id);
    return name;
}

struct promise* new_promise(void)
{
    struct promise* new = task_malloc(sizeof(struct promise));
    memset(new, 0, sizeof(struct promise));
    return new;
}

static jerry_value_t null_function(const jerry_value_t function_obj_val,
                                   const jerry_value_t this_val,
                                   const jerry_value_t args_p[],
                                   const jerry_length_t args_cnt)
{
    PRINT("NULL FUNCTION\n");
    return ZJS_UNDEFINED;
}

static jerry_value_t* pre_then(void* h, uint32_t* args_cnt)
{
    PRINT("pre_then()\n");
    struct promise* handle = (struct promise*)h;
    jerry_value_t global_obj = jerry_get_global_object();
    jerry_value_t promise_obj = zjs_get_property(global_obj, make_name(handle->id, "promise"));

    *args_cnt = handle->then_args_cnt;
    return handle->then_args;
}

static jerry_value_t promise_then(const jerry_value_t function_obj_val,
                                   const jerry_value_t this_val,
                                   const jerry_value_t args_p[],
                                   const jerry_length_t args_cnt)
{
    PRINT("In promise_then()\n");
    //jerry_value_t promise_obj = zjs_get_property(this_val, "promise");
    struct promise* handle;

    jerry_get_object_native_handle(this_val, (uintptr_t*)&handle);

    if (jerry_value_is_function(args_p[0])) {

        handle->then = jerry_acquire_value(args_p[0]);
        PRINT("promise_then(): handle=%p, id=%u, saving function %p\n", handle, handle->id, handle->then);
        zjs_edit_js_func(handle->then_id, handle->then);
        return this_val;
    } else {
        return ZJS_UNDEFINED;
    }
}

static jerry_value_t* pre_catch(void* h, uint32_t* args_cnt)
{
    struct promise* handle = (struct promise*)h;
    jerry_value_t global_obj = jerry_get_global_object();
    jerry_value_t promise_obj = zjs_get_property(global_obj, make_name(handle->id, "promise"));

    *args_cnt = handle->catch_args_cnt;
    return handle->catch_args_cnt;
}

static jerry_value_t promise_catch(const jerry_value_t function_obj_val,
                                   const jerry_value_t this_val,
                                   const jerry_value_t args_p[],
                                   const jerry_length_t args_cnt)
{
    jerry_value_t promise_obj = zjs_get_property(this_val, "promise");
    struct promise* handle;

    jerry_get_object_native_handle(promise_obj, (uintptr_t*)&handle);

    if (jerry_value_is_function(args_p[0])) {
        handle->catch = jerry_acquire_value(args_p[0]);
        return this_val;
    } else {
        return ZJS_UNDEFINED;
    }
}

int32_t zjs_make_promise(jerry_value_t obj)
{
    struct promise* new = new_promise();
    jerry_value_t promise_obj = jerry_acquire_value(jerry_create_object());
    jerry_value_t global_obj = jerry_acquire_value(jerry_get_global_object());

    zjs_obj_add_function(obj, promise_then, "then");
    zjs_obj_add_function(obj, promise_catch, "catch");
    jerry_set_object_native_handle(obj, new, NULL);
    //zjs_set_property(obj, promise_obj, "promise");

    new->id = find_id(&use);

    char* name = make_name(new->id, "promise");

    zjs_obj_add_object(global_obj, obj, name);

    PRINT("Creating promise for object, id %u, handle=%p, name=%s\n", new->id, new, name);

    jerry_release_value (global_obj);

    return new->id;
}

void zjs_fulfill_promise(int32_t id, jerry_value_t args[], uint32_t args_cnt)
{
    struct promise* handle;
    uintptr_t h;
    jerry_value_t global_obj = jerry_get_global_object();

    char* name = make_name(id, "promise");

    PRINT("Getting global name: %s\n", name);

    jerry_value_t promise_obj = zjs_get_property(global_obj, name);

    jerry_get_object_native_handle(promise_obj, (uintptr_t*)&h);
    handle = h;



    handle->then = jerry_acquire_value(jerry_create_external_function(null_function));

    handle->then_id = zjs_add_callback(handle->then, handle, pre_then, NULL);

    PRINT("THEN FUNC = %p\n", handle->then);

    handle->then_args = args;
    handle->then_args_cnt = args_cnt;

    PRINT("then_args=%p\n", handle->then_args);

    PRINT("Fulfilling promise %u, callback id %u, handle=%p\n", handle->id, handle->then_id, handle);

    zjs_signal_callback(handle->then_id);

    PRINT("Signaled callback\n");
}

void zjs_reject_promise(int32_t id, jerry_value_t args[], uint32_t args_cnt)
{
    struct promise* handle;
    jerry_value_t global_obj = jerry_get_global_object();
    jerry_value_t promise_obj = zjs_get_property(global_obj, make_name(id, "promise"));
    jerry_get_object_native_handle(promise_obj, (uintptr_t*)&handle);

    handle->catch_id = zjs_add_callback(handle->catch, handle, pre_catch, NULL);

    handle->catch_args = args;
    handle->catch_args_cnt = args_cnt;

    PRINT("Rejecting promise %u, callback id %u\n", handle->id, handle->catch_id);

    zjs_signal_callback(handle->catch_id);
}

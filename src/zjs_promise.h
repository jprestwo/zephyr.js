#ifndef __zjs_promises_h__
#define __zjs_promises_h__

#include "zjs_util.h"

int32_t zjs_make_promise(jerry_value_t obj);

void zjs_fulfill_promise(int32_t id, jerry_value_t args[], uint32_t args_cnt);

void zjs_reject_promise(int32_t id, jerry_value_t args[], uint32_t args_cnt);

#endif /* __zjs_promises_h__ */

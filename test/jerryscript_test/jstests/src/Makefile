subdir-ccflags-y += -D__ZEPHYR__
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE_DISABLE_ARRAY_BUILTIN
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE_DISABLE_BOOLEAN_BUILTIN
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE_DISABLE_ERROR_BUILTINS
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE_DISABLE_JSON_BUILTIN
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE_DISABLE_MATH_BUILTIN
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE_DISABLE_NUMBER_BUILTIN
subdir-ccflags-y += -DCONFIG_ECMA_COMPACT_PROFILE_DISABLE_STRING_BUILTIN
subdir-ccflags-y += -DJERRY_ENABLE_SNAPSHOT_EXEC
subdir-ccflags-y += -DJERRY_ENABLE_SNAPSHOT_SAVE
subdir-ccflags-y += -DJERRY_NDEBUG
subdir-ccflags-y += -DJERRY_DISABLE_HEAVY_DEBUG
subdir-ccflags-y += -DMEM_STATS
subdir-ccflags-y += -DCONFIG_MEM_HEAP_AREA_SIZE="22768"

obj-y += ../../../../deps/jerryscript/jerry-core/

ccflags-y += -I$(src)/../../../../deps/jerryscript/jerry-core

obj-y += main.o

obj-y += ../../../../deps/jerryscript/jerry-core/lib.a

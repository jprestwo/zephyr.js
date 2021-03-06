ifndef JERRY_BASE
$(error JERRY_BASE not defined)
endif

MDEF_FILE = prj.mdef
KERNEL_TYPE = micro
BOARD ?= arduino_101_factory
CONF_FILE ?= prj.conf

VARIETY ?= -cp_minimal

KBUILD_ZEPHYR_APP = librelease${VARIETY}.jerry-core.a

obj-y += src/

export KBUILD_ZEPHYR_APP BOARD VARIETY

include ${ZEPHYR_BASE}/Makefile.inc

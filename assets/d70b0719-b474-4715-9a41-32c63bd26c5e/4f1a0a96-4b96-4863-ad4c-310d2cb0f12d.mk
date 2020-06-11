LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := injector
LOCAL_SRC_FILES := main.c utils.c
LOCAL_LDFLAGS += -pie -ldl

include $(BUILD_EXECUTABLE)

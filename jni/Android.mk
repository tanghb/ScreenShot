LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libfb2png_static
LOCAL_SRC_FILES := \
    fb2png.c \
    img_process.c \
    fb.c

LOCAL_C_INCLUDES +=\
    external/libpng\
    external/zlib

LOCAL_CFLAGS += -DANDROID
LOCAL_STATIC_LIBRARIES := libpng libz

include $(BUILD_STATIC_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE    := libsave
LOCAL_SRC_FILES := screenshot.c
LOCAL_LDLIBS := -llog
LOCAL_C_INCLUDES  += system/core/include/cutils
LOCAL_STATIC_LIBRARIES := libcutils libfb2png_static libpng libz libc

LOCAL_C_INCLUDES +=\
    external/libpng\
    external/zlib

include $(BUILD_SHARED_LIBRARY)


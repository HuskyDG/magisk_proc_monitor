LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := am_proc_start
LOCAL_SRC_FILES := am_proc_start.cpp base.cpp
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)

include jni/libcxx/Android.mk
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := proc_monitor
LOCAL_SRC_FILES := am_proc_start.cpp base.cpp cus.cpp zygisk.cpp
LOCAL_STATIC_LIBRARIES := libcxx
LOCAL_LDLIBS := -llog
LOCAL_LDFLAGS := -Wl,--dynamic-list=jni/exported_sym.txt
include $(BUILD_EXECUTABLE)

include jni/libcxx/Android.mk

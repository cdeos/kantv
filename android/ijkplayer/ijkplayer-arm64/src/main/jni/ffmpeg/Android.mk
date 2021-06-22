LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := iconv
LOCAL_SRC_FILES := $(MY_APP_ICONV_OUTPUT_PATH)/lib/libiconv.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_ICONV_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_ICONV_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := charset
LOCAL_SRC_FILES := $(MY_APP_ICONV_OUTPUT_PATH)/lib/libcharset.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_ICONV_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_ICONV_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := xml2
LOCAL_SRC_FILES := $(MY_APP_XML2_OUTPUT_PATH)/lib/libxml2.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_XML2_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_XML2_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := ssl
LOCAL_SRC_FILES := $(MY_APP_OPENSSL_OUTPUT_PATH)/lib/libssl.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_OPENSSL_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_OPENSSL_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := crypto
LOCAL_SRC_FILES := $(MY_APP_OPENSSL_OUTPUT_PATH)/lib/libcrypto.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_OPENSSL_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_OPENSSL_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := curl
LOCAL_SRC_FILES := $(MY_APP_CURL_OUTPUT_PATH)/lib/libcurl.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_CURL_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_CURL_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := drmclient
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libdrmclient.so
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := tensorflowlite_c
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libtensorflowlite_c.so
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_SHARED_LIBRARY)


include $(CLEAR_VARS)
LOCAL_MODULE    := avcodec
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavcodec.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avformat
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavformat.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := swscale
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libswscale.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_OUTPUT_PATH)/include
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avutil
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavutil.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE    := avfilter
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libavfilter.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
#LOCAL_MODULE := ijkffmpeg
#LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/libijkffmpeg.so
#include $(PREBUILT_SHARED_LIBRARY)

LOCAL_MODULE    := swresample
LOCAL_SRC_FILES := $(MY_APP_FFMPEG_OUTPUT_PATH)/lib/libswresample.a
LOCAL_EXPORT_C_INCLUDES := $(MY_APP_FFMPEG_INCLUDE_PATH)
LOCAL_C_INCLUDES += $(MY_APP_FFMPEG_INCLUDE_PATH)
include $(PREBUILT_STATIC_LIBRARY)

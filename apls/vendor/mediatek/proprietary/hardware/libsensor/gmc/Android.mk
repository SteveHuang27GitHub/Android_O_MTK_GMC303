LOCAL_PATH:= $(call my-dir)

GME_FS_LIB=libGME_OSS
export GMED_DEVICE_TYPE=303
#export GMED_DEVICE_TYPE=306
#export GMED_DEVICE_TYPE=605
##### GME daemon ###############################################################
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := \
        $(MTK_PATH_SOURCE)/hardware/sensor/ \
        $(MTK_PATH_CUSTOM)/hal/sensors/sensor \
        $(MTK_PATH_SOURCE)/hardware/sensor/sensors-1.0/include \
	$(LOCAL_PATH)/$(GME_FS_LIB)

LOCAL_SRC_FILES:= \
	$(GME_FS_LIB)/GMFS_AOC.c \
	$(GME_FS_LIB)/GMFS_Decomp.c \
	$(GME_FS_LIB)/GMFS_Device.c \
	$(GME_FS_LIB)/GMFS_Direction.c \
	$(GME_FS_LIB)/GMFS_VNorm.c \
	$(GME_FS_LIB)/ossf_magnetic.c \
	GMFS_APIs.c \
	GMFS_Measure.c \
	HAL_gmadfs306.c

LOCAL_CFLAGS += -Wall
LOCAL_CFLAGS += -DGMFS_OUTPUT_AVEC
LOCAL_CFLAGS += -DGME_VALUE_CHECK
LOCAL_CFLAGS += -DENABLE_GMEDEBUG=1
LOCAL_MULTILIB := both

ifeq ($(GMED_DEVICE_TYPE), 303)
LOCAL_CFLAGS += -DGME_DEVICE_GMC303
endif

ifeq ($(GMED_DEVICE_TYPE), 306)
LOCAL_CFLAGS += -DGME_DEVICE_GMC306
endif

ifeq ($(GMED_DEVICE_TYPE), 605)
LOCAL_CFLAGS += -DGME_DEVICE_GME605
endif

#LOCAL_MODULE := gmadfs306
LOCAL_MODULE := libgmc
LOCAL_MODULE_TAGS := optional
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
#LOCAL_FORCE_STATIC_EXECUTABLE := false
LOCAL_SHARED_LIBRARIES := libc libm libcutils liblog

include $(MTK_SHARED_LIBRARY)


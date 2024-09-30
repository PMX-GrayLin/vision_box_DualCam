# TOPDIR is where phoenix_genericlinux/ is located
#include ../config.mk

# Version
VERSION_FLAG=
#VERSION_FLAG += -DFW_VERSION="\"3.01.01\"" 
VERSION_FLAG += -DVSB_VERSION="\"IPS.0.0.4\"" #IPS_version : 10/11/2023 : RERVE.VERSION.PATCHLEVE.WEEKNUM
VERSION_FLAG += -DIOS_VERSION="\"IOS.0.0.4\""
VERSION_FLAG += -DALGO_Enable_MAINLOG_DEBUG
VERSION_FLAG += -DALGO_Enable_IPSLOG_DEBUG
VERSION_FLAG += -DALGO_Enable_REXTYLOG_DEBUG
# VERSION_FLAG += -DALGO_Enable_CYCLE_TIME_DEBUG
VERSION_FLAG += -DALGO_Enable_MeasGlueWidth_ResultIMG_DEBUG
# VERSION_FLAG += -DALGO_Enable_ImgBufOpt_InputImagDump_DEBUG
# VERSION_FLAG += -DALGO_Enable_ImgBufOpt_AddTimestamp_DEBUG
VERSION_FLAG += -DALGO_Enable_StreamingBufOpt_AddTimestamp_DEBUG
VERSION_FLAG += -DALGO_Enable_StreamingBufOpt_SpeedOptimization_DEBUG
VERSION_FLAG += -DALGO_Enable_StreamingBufOpt_EnableGStreamer_DEBUG

# GG
# PP = aarch64-linux-gnu-g++
PP = ${CXX}
CXXFLAGS += "-std=c++11"

HOMEPATH=~/rextyw/vision_box-merge/ipsCtl/Merge_Measure_GlueWidth
		
TARGET=vision_box_DualCam

# Debug print format FLAG
DBG_PRINT_FLAG=
DBG_PRINT_FLAG += -DENB_VSB_AUTORUN_MODE

# Debug FLAG
DFLAG= 
DFLAG += $(DBG_PRINT_FLAG) 
DFLAG += $(VERSION_FLAG)

# CPP compile FLAG
CPPFLAG=
CPPFLAG += -std=c++14 -O0 -g 
CPPFLAG += -Wall -Wcomment -pipe -MMD
CPPFLAG += -fpermissive -Wwrite-strings -Wreturn-type -Wunused-variable
CPPFLAG += -I/usr/include/modbus

# Include 
INCLUDE= 
INCLUDE +=-I .
INCLUDE +=-I $(shell pwd)
INCLUDE +=-I $(shell pwd)/ipsCtl/Merge_Measure_GlueWidth
INCLUDE +=-I $(shell pwd)/ipsCtl/Merge_Measure_GlueWidth/GigECam
INCLUDE +=-I $(shell pwd)/ipsCtl/Merge_Measure_GlueWidth/PlugIn
INCLUDE +=-I $(shell pwd)/ipsCtl/Merge_Measure_GlueWidth/ThirdPartyLibrary
INCLUDE +=-I /usr/include
INCLUDE +=-I /usr/include/opencv2
INCLUDE +=-I /home/user/primax/include/opencv4
INCLUDE +=-I /usr/include/spinnaker
INCLUDE +=-I /usr/include/glib-2.0
INCLUDE +=-I /usr/lib/aarch64-linux-gnu/glib-2.0/include
INCLUDE +=-I /usr/local/include
INCLUDE +=-I /usr/include/json-c
INCLUDE +=-I /usr/local/include/json-c
INCLUDE +=-I /usr/local/include/modbus
INCLUDE +=-I /usr/include/python3.10
INCLUDE +=-I /home/user/.local/lib/python3.10/site-packages/numpy/core/include
INCLUDE +=-I /home/user/.local/lib/python3.10/site-packages/numpy/core/include/numpy
INCLUDE +=-I /opt/MVS/include
INCLUDE +=-I /usr/include/curl
INCLUDE +=-I /opt/pylon/include
INCLUDE +=-I /opt/OPT/OPTCameraDemo/include
INCLUDE +=-I /opt/spinnaker/include

INCLUDE += -I$(shell pwd)/iosCtl/i2c-tools/tools
INCLUDE += -I$(shell pwd)/iosCtl/i2c-tools/include
INCLUDE += -I$(shell pwd)/iosCtl/tof_lib
INCLUDE += -I$(shell pwd)/iosCtl/tof_lib/core

INCLUDE +=-I/home/ubuntu/primax/image/usr/include/opencv4
INCLUDE +=-I/home/ubuntu/primax/image/usr/include/neuron/api

INCLUDE +=-I /home/ubuntu/primax/image/usr/include
LDFLAG += -L /home/ubuntu/primax/image/usr/lib


LDFLAG= 
LDFLAG += -L -ldl -lc -lm -lrt -lpthread
LDFLAG += -L /usr/local/lib/ -L /usr/lib/aarch64-linux-gnu/ -lmosquitto -ljson-c -lpython3.10 -lcurl
LDFLAG += -L/usr/lib -lmodbus
LDFLAG += $(shell pwd)/iosCtl/i2c-tools/tools/i2cbusses.o
LDFLAG += $(shell pwd)/iosCtl/i2c-tools/tools/util.o
LDFLAG += $(shell pwd)/iosCtl/i2c-tools/lib/smbus.o
LDFLAG += $(shell pwd)/iosCtl/tof_lib/vl53l1_linux_platform.o
LDFLAG += $(shell pwd)/iosCtl/tof_lib/core/VL53L1X_api.o

################################################################################
# Dependencies
################################################################################

# OpenCV Library FLAG
OCVLDFLAG = -Wl,-Bdynamic
OCVLDFLAG +=-L /usr/local/lib/opencv451 -L/home/user/primax/lib
OCVLDFLAG +=-lopencv_core 
OCVLDFLAG +=-lopencv_imgproc
OCVLDFLAG +=-lopencv_features2d
OCVLDFLAG +=-lopencv_video 
OCVLDFLAG +=-lopencv_videoio 
OCVLDFLAG +=-lopencv_imgcodecs 
OCVLDFLAG +=-lopencv_objdetect 
OCVLDFLAG +=-lopencv_highgui 
OCVLDFLAG +=-lopencv_photo 
OCVLDFLAG +=-lopencv_flann
OCVLDFLAG +=-lopencv_ml
OCVLDFLAG +=-lopencv_dnn

# Spinnaker deps
# SPINNAKER_LIB = -Wl,-Bdynamic
# SPINNAKER_LIB +=-L /usr/local/lib/spinnaker140
# SPINNAKER_LIB +=-L /opt/spinnaker/lib
# SPINNAKER_LIB +=-lSpinnaker

# HikVision deps
# HIK_LIB = -Wl,-Bdynamic
# HIK_LIB +=-L /opt/MVS/lib/aarch64
# HIK_LIB +=-lFormatConversion
# HIK_LIB +=-lMVGigEVisionSDK
# HIK_LIB +=-lMvCameraControl
# HIK_LIB +=-lMVRender
# HIK_LIB +=-lMediaProcess

# Baseler(pylon) deps
# BASLER_LIB = -Wl,-Bdynamic
# BASLER_LIB +=-L /opt/pylon/lib -L/opt/OPT/OPTCameraDemo/lib/GenICam/bin/
# BASLER_LIB +=-lpylonbase
# BASLER_LIB +=-lpylonutility
# BASLER_LIB +=-lGenApi_gcc_v3_1_Basler_pylon
# BASLER_LIB +=-lGCBase_gcc_v3_1_Basler_pylon
# BASLER_LIB +=-lGenApi_gcc485_v3_0

# OPT deps
# OPT_LIB = -Wl,-Bdynamic
# OPT_LIB +=-L /opt/OPT/OPTCameraDemo/lib
# OPT_LIB +=-lOPTSDK
# OPT_LIB +=-lImageConvert
# OPT_LIB +=-llog4cpp
# OPT_LIB +=-lRecordVideo

CSOURCEFILE = $(wildcard *.c)
COBJECTS = $(patsubst %.c,%.o,$(CSOURCEFILE))

CPPSOURCEFILE = $(wildcard *.cpp)
CPPOBJECTS = $(patsubst %.cpp,%.o,$(CPPSOURCEFILE))

MAIN_SRC = $(wildcard mainCtl/*.c)
MAIN_OBJS = $(patsubst %.c,%.o,$(MAIN_SRC))

IOS_SRC = $(wildcard iosCtl/*.c)
IOS_OBJS = $(patsubst %.c,%.o,$(IOS_SRC))

IPS_SRC = $(wildcard ipsCtl/*.c)
IPS_OBJS = $(patsubst %.c,%.o,$(IPS_SRC))

SRC_IPL = $(wildcard ipsCtl/Merge_Measure_GlueWidth/*.cpp)
OBJ_IPL = $(patsubst %.cpp,%.o,$(SRC_IPL))

SRC_GIGE = $(wildcard ipsCtl/Merge_Measure_GlueWidth/GigECam/*.cpp)
OBJ_GIGE = $(patsubst %.cpp,%.o,$(SRC_GIGE))

SRC_TPL = $(wildcard ipsCtl/Merge_Measure_GlueWidth/ThirdPartyLibrary/*.cpp)
OBJ_TPL = $(patsubst %.cpp,%.o,$(SRC_TPL))

# all: i2ctools tof_lib $(CPPOBJECTS) $(COBJECTS) $(MAIN_OBJS) $(IOS_OBJS) $(IPS_OBJS) $(OBJ_IPL) $(OBJ_GIGE) $(OBJ_MLDL) $(OBJ_TPL)
# 	@echo ""
# 	$(PP) $(DFLAG) $(CPPFLAG) $(CPPOBJECTS) $(COBJECTS) $(MAIN_OBJS) $(IOS_OBJS) $(IPS_OBJS) $(OBJ_IPL) $(OBJ_GIGE) $(OBJ_MLDL) $(OBJ_TPL) $(LDFLAG) $(OCVLDFLAG) $(SPINNAKER_LIB) $(ARAVIS_LIB) $(HIK_LIB) $(BASLER_LIB) $(OPT_LIB) -o $(TARGET) 
# 	ls -l $(TARGET)

all: i2ctools tof_lib $(IOS_OBJS)

i2ctools:
	(cd iosCtl/i2c-tools/; make)

tof_lib:
	(cd iosCtl/tof_lib/; make clean; make)

%.o: $(SRC_TPL)%.cpp
	@echo ""
	@echo Compiling $< ...
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) -c $< -o $@

%.o: $(SRC_GIGE)%.cpp
	@echo ""
	@echo Compiling $< ...
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) -c $< -o $@

%.o: $(SRC_IPL)%.cpp
	@echo ""
	@echo Compiling $< ...
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) -c $< -o $@

%.o: $(SRC_MLDL)%.cpp
	@echo ""
	@echo Compiling $< ...
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) -c $< -o $@

%.o: %.cpp
	@echo ""
	@echo Compiling $< ...
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) -c $< -o $@

%.o: %.c
	@echo ""
	@echo Compiling $< ...
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) -c $< -o $@

clean:
	rm -f *.o *.d *.opp $(TARGET) $(CPPOBJECTS) $(COBJECTS) $(MAIN_OBJS) $(IOS_OBJS) $(IPS_OBJS) $(OBJ_IPL) $(OBJ_GIGE) $(OBJ_MLDL) $(OBJ_TPL)
	(cd iosCtl/i2c-tools/; make clean)
	(cd iosCtl/tof_lib/; make clean)

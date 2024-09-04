#Version
VERSION_FLAG=
VERSION_FLAG += -DIPLDLL_VERSION="\"1.01.01\"" #04/207/2022 


TARGET=Unittesting


PP = aarch64-linux-gnu-g++


HOMEPATH=~/rextyw/vision_box_rexty/ipsCtl/Merge_Measure_GlueWidth


#Debug print format FLAG
DBG_PRINT_FLAG=
DBG_PRINT_FLAG += -DALL_MSG_DEBUG -DELAPSED_TIME_ms_DEBUG
#DBG_PRINT_FLAG += -DALL_MSG_DEBUG -DALGO_ENTRANCE_DEBUG -DELAPSED_TIME_ms_DEBUG
#DBG_PRINT_FLAG += -DALL_MSG_DEBUG -DALGO_ENTRANCE_DEBUG -DELAPSED_TIME_ms_DEBUG -D_RexTY_APP_DEBUG


#Debug FLAG
DFLAG= 
DFLAG += $(DBG_PRINT_FLAG) $(VERSION_FLAG)
#DFLAG += -DIVS
#DFLAG += -DALL_MSG_DEBUG           # All Debug Message setting.
#DFLAG += $(DBG_PRINT_FLAG) $(VERSION_FLAG)


#CPP compile FLAG
CPPFLAG=
CPPFLAG += -std=c++14 -O2 -ldl -lc -lm -lrt -lz -lpthread
#CPPFLAG += -std=c++14 -Wall -g -O2 -ldl -lc -lm -lrt -lz -lpthread
#CPPFLAG +=-std=c++14 -fPIC -c 
#CPPFLAG +=-Wall -g -O2 -ldl -lm -lrt -std=c++14 -pthread 
#CPPFLAG +=-lpthread -mcpu=cortex-a9 -march=armv7-a -mtune=cortex-a9 -mthumb -mfpu=neon 
#CPPFLAG +=-ftree-vectorize -fopenmp


#Include 
INCLUDE= 
INCLUDE +=-I .
INCLUDE +=-I $(HOMEPATH)
INCLUDE +=-I $(HOMEPATH)/GigECam
INCLUDE +=-I /usr/include
INCLUDE +=-I /usr/include/opencv2
INCLUDE +=-I /usr/include/spinnaker
#INCLUDE +=-I /opt/spinnaker/include
#INCLUDE +=-I /opt/spinnaker/include/spinnaker


#LD FLAG
LDFLAG= 
#LDFLAG += -L .
#LDFLAG += -lIPLDLL
#LDFLAG +=-I../../forIVS/libjpeg_turbo/include -L../../forIVS/libjpeg_turbo/lib/ -lturbojpeg
#LDFLAG +=-I$(HOMEPATH)/packages/data_transfer -L$(HOMEPATH)/out/s2ipcam/packages/data_transfer -ldatatx
#LDFLAG +=-I$(HOMEPATH)/packages/dewarp/arch_s2 -L$(HOMEPATH)/prebuild/dewarp -ldewarp 
#LDFLAG +=-I$(HOMEPATH)/packages/utils -L$(HOMEPATH)/out/s2ipcam/packages/utils -lutils 


################################################################################
# Dependencies
################################################################################
# Spinnaker deps
SPINNAKER_LIB =
SPINNAKER_LIB = -Wl,-Bdynamic
SPINNAKER_LIB +=-L /usr/local/lib/spinnaker140
#SPINNAKER_LIB +=-L /opt/spinnaker/lib
SPINNAKER_LIB +=-lSpinnaker


#OpneCV Library FLAG
# -libopencv_stitching -libopencv_calib3d
OCVLDFLAG= -Wl,-Bdynamic
OCVLDFLAG +=-L /usr/local/lib/opencv451
OCVLDFLAG +=-lopencv_core 
OCVLDFLAG +=-lopencv_gapi 
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




#SOURCEFILE=$(shell echo *.cpp)
SOURCEFILE = $(wildcard *.cpp)
CPPOBJECTS = $(patsubst %.cpp,%.opp,$(SOURCEFILE))

SRC_GIGE = $(wildcard ./GigECam/*.cpp)
OBJ_GIGE = $(patsubst %.cpp,%.opp,$(SRC_GIGE))

all: $(CPPOBJECTS) $(OBJ_GIGE)
	@echo ""
	$(PP) $(DFLAG) $(CPPFLAG) $(CPPOBJECTS) $(OBJ_GIGE) $(LDFLAG) $(OCVLDFLAG) $(SPINNAKER_LIB) -o $(TARGET) 
	ls -l $(TARGET)
	
%.opp: $(SRC_GIGE)%.cpp
	@echo ""
	@echo Compiling $< ... 
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) $(LDFLAG) $(OCVLDFLAG) $(SPINNAKER_LIB) -c $< -o $@ 	
	
%.opp: %.cpp
	@echo ""
	@echo Compiling $< ... 
	$(PP) $(DFLAG) $(CPPFLAG) $(INCLUDE) $(LDFLAG) $(OCVLDFLAG) $(SPINNAKER_LIB) -c $< -o $@ 

.PHONY : clean	
clean:
	rm -f *.opp $(TARGET) $(CPPOBJECTS) $(OBJ_GIGE)
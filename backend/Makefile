# TOPDIR is where phoenix_genericlinux/ is located
#CC=aarch64-linux-gnu-gcc
CC=aarch64-linux-gnu-g++
include ../config.mk

DFLAG=
DFLAG += -DENB_VSB_AUTORUN_MODE

BACKEND = backend
BACKEND_GI = backend_gi
BACKEND_SRC = backend.c
BACKEND_OBJ = backend.o
BACKEND_GI_OBJ = backend_gi.o
MQTT = mqtt
MQTT_SRC = mqtt_client.c
MQTT_OBJ = mqtt_client.o
GPIOCTRL = gpioctrl
GPIOCTRL_SRC = gpioctrl.c
GPIOCTRL_OBJ = gpioctrl.o


CFLAGS=-Wall -Wcomment -O0 -g -pipe -MMD -fpermissive -I/usr/include -I/usr/local/include/json-c -I/usr/include/json-c 
LDFLAGS+=-L/usr/local/lib -L/usr/lib/aarch64-linux-gnu /usr/lib/aarch64-linux-gnu/libmosquitto.so
#LDFLAGS+=-L/usr/local/lib -lmosquitto -lgpiod
#LDFLAGS+=-L/lib/aarch64-linux-gnu -lrt -pthread -ljson-c 
LDFLAGS+=-L -lrt -pthread -ljson-c 


all: $(BACKEND)  $(BACKEND_GI)
	@echo ""
	ls -l $(BACKEND)

$(MQTT_OBJ): $(MQTT_SRC) 
	$(CC) $(DFLAG) $(CFLAGS) -c $< -o $@
  
$(GPIOCTRL_OBJ): $(GPIOCTRL_SRC) 
	$(CC) $(DFLAG) $(CFLAGS) -c $< -o $@   
  
$(BACKEND): $(BACKEND_OBJ) $(GPIOCTRL_OBJ) $(MQTT_OBJ) $(MQTT_LIB) /usr/lib/aarch64-linux-gnu/libjson-c.so
	$(CC) $(DFLAG) $(CFLAGS) $(LDFLAGS) -o $(BACKEND) $^

$(BACKEND_GI): $(BACKEND_GI_OBJ) $(GPIOCTRL_OBJ) $(MQTT_OBJ) $(MQTT_LIB) /usr/lib/aarch64-linux-gnu/libjson-c.so
	$(CC) $(DFLAG) $(CFLAGS) $(LDFLAGS) -o $(BACKEND_GI) $^

clean:
	rm -f *.o *.d $(BACKEND) $(BACKEND_GI)

.PHONY: all clean

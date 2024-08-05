#
# Makefile
#-L/home/gec/freetype-2.12.1/tmp/lib -I/home/gec/freetype-2.12.1/tmp/include/freetype2
#CC = gcc
CC = /usr/bin/aarch64-linux-gnu-gcc
LVGL_DIR_NAME ?= lvgl
LVGL_DIR ?= ${shell pwd}
CFLAGS ?= -O3 -g0   -I$(LVGL_DIR)/ -Wall  -std=gnu99
LDFLAGS ?= -lm#数学库是标准C库的一部分，因此不需要-L指定库文件路径
BIN = demo

#我自己的
#画板相关
CFLAGS += -I./user 
#链接库的路径
# LDFLAGS += -L/usr/aarch64-linux-gnu/lib
#链接库
# LDFLAGS += -lcurl

# 使用 scp 命令将文件发送到目标板子
EXECUTABLE := demo
TARGET_IP := 192.168.10.108
TARGET_USER := cat
TARGET_PATH := /home/cat/demo
PASSWORD := temppwd

#Collect the files to compile
MAINSRC = ./main.c

include $(LVGL_DIR)/lvgl/lvgl.mk
include $(LVGL_DIR)/lv_drivers/lv_drivers.mk

CSRCS +=$(LVGL_DIR)/mouse_cursor_icon.c 

#我自己的
CSRCS += $(LVGL_DIR)/user/lv_100ask_sketchpad.c
CSRCS += $(LVGL_DIR)/user/cJSON.c
CSRCS += $(LVGL_DIR)/user/tip.c

OBJEXT ?= .o

AOBJS = $(ASRCS:.S=$(OBJEXT))
COBJS = $(CSRCS:.c=$(OBJEXT))

#我自己的
#COBJS += $(patsubst %.c,%$(OBJEXT),$(wildcard user/*.c))
# COBJS := $(patsubst %.c,%$(OBJEXT),$(wildcard user/*.c))

MAINOBJ = $(MAINSRC:.c=$(OBJEXT))

SRCS = $(ASRCS) $(CSRCS) $(MAINSRC)
OBJS = $(AOBJS) $(COBJS)

## MAINOBJ -> OBJFILES


all: default

%.o: %.c
	@$(CC)  $(CFLAGS) -c $< -o $@
	@echo "CC $<"
    
default: $(AOBJS) $(COBJS) $(MAINOBJ)
	$(CC) -o $(BIN) $(MAINOBJ) $(AOBJS) $(COBJS) $(LDFLAGS)

clean: 
	rm -f $(BIN) $(AOBJS) $(COBJS) $(MAINOBJ)

send:
	sshpass -p $(PASSWORD) scp $(BIN) $(TARGET_USER)@$(TARGET_IP):$(TARGET_PATH)

run:
	sshpass -p $(PASSWORD) ssh $(TARGET_USER)@$(TARGET_IP) "cd $(TARGET_PATH); sudo ./$(EXECUTABLE)" | tail -f

stop:
	sshpass -p $(PASSWORD) ssh $(TARGET_USER)@$(TARGET_IP) "sudo killall -9 $(EXECUTABLE)"

sed:
	scp $(BIN) $(TARGET_USER)@$(TARGET_IP):$(TARGET_PATH)
	
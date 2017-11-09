

#export CROSS:=arm-linux-
export CROSS:=
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar

DVRPATH = ./..

LIBPATH = $(DVRPATH)/lib

CFLAGS = -Wall -O -g -DLINUX


LIBS := $(LIBPATH)/utility.a

INC_PATH = -I$(DVRPATH)/include

TARGET = ./main

SRC = main.c fota_analyze.c tcpserver.c v1_data_analyze.c 

OBJS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRC)))

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $< -o $@

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(LIBS) -lpthread -lm

	chmod a+x $(TARGET)
	@rm *~ -f
clean:
	rm -f $(TARGET) $(OBJS)





















ARM_LINUX = Y
ifeq ($(ARM_LINUX), Y)
export CROSS:=arm-linux-
else
export CROSS:=
endif
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar

DVRPATH = ./..

LIBPATH = $(DVRPATH)/lib/app

CFLAGS = -Wall -O -g -DLINUX

LIBS := $(LIBPATH)/utility.a

INC_PATH = -I$(DVRPATH)/include/microwin \
		-I$(DVRPATH)/include/app 

TARGET = ./main

SRC = main.c

OBJS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRC)))

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $< -o $@

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -o $@ $^ $(LIBS) -lpthread -lm

	chmod a+x $(TARGET)
	@rm *~ -f
	cp main ../release/

clean:
	rm -f $(TARGET) $(OBJS)




















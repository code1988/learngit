ARM_LINUX = Y
ifeq ($(ARM_LINUX), Y)
export CROSS:=arm-linux-
else
export CROSS:=
endif
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar

DVRPATH = ./..

LIBPATH = $(DVRPATH)/lib

CFLAGS = -Wall -O -g -DLINUX -DFOTA_KEY
LIBS := $(LIBPATH)/microwin/libmwin.a $(LIBPATH)/microwin/libmwinlib.a \
	$(LIBPATH)/microwin/libmwengine.a $(LIBPATH)/microwin/libmwdrivers.a \
	$(LIBPATH)/microwin/libmwfonts.a $(LIBPATH)/microwin/libmwimages.a \
	$(LIBPATH)/microwin/libmwobjects.a $(LIBPATH)/app/drvlib.a \
	$(LIBPATH)/app/fota.a $(LIBPATH)/app/monitor.a \
	$(LIBPATH)/app/libsqlite3.a $(LIBPATH)/app/utility.a \
	$(LIBPATH)/app/libDeca.a

INC_PATH = -I$(DVRPATH)/include/microwin \
		-I$(DVRPATH)/include/app
TARGET = ./appwin

SRC = $(wildcard *.c)
 

OBJS = $(patsubst %.c, %.o, $(SRC))

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $< -o $@

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -lrt -ldl -o $@ $^ $(LDFLAGS) $(LIBS) -lpthread -lm

	chmod a+x $(TARGET)
	@rm *~ -f
	cp appwin ../release/
	cp appwin /tftpboot/
clean:
	rm -f $(TARGET) $(OBJS)




















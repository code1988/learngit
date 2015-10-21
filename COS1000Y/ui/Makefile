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

LIBS := $(LIBPATH)/microwin/libmwin.a \
	$(LIBPATH)/microwin/libmwinlib.a \
	$(LIBPATH)/microwin/libmwengine.a \
	$(LIBPATH)/microwin/libmwdrivers.a \
	$(LIBPATH)/microwin/libmwfonts.a \
	$(LIBPATH)/microwin/libmwimages.a \
	$(LIBPATH)/microwin/libmwin.a \
	$(LIBPATH)/microwin/libmwinlib.a\
	$(LIBPATH)/microwin/libmwengine.a \
	$(LIBPATH)/microwin/libmwdrivers.a \
	$(LIBPATH)/microwin/libmwfonts.a \
	$(LIBPATH)/microwin/libmwimages.a \
	$(LIBPATH)/app/drvlib.a \
	$(LIBPATH)/app/fota.a \
	$(LIBPATH)/app/monitor.a \
	$(LIBPATH)/app/drvlib.a \
	$(LIBPATH)/app/fota.a \
	$(LIBPATH)/app/monitor.a \
	$(LIBPATH)/app/libsqlite3.a \
	$(LIBPATH)/app/utility.a 

INC_PATH = -I$(DVRPATH)/include/microwin \
		-I$(DVRPATH)/include/app
TARGET = ./fotawin

SRC = main.c Menu.c Paraset.c myIP.c myMask.c myPort.c myMAC.c myGate.c serverIP.c serverPort.c \
	  countmodewin.c Netnumber.c Password.c Timeset.c Blackname.c Batchset.c Agetest.c \
	  identifywin.c levelsetwin.c Version.c Adjustmenu.c guanzihaowin.c Functionset.c Uploadimg.c \
	  Datamanage.c dynamicwin.c staticwin.c


OBJS = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRC)))

all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $< -o $@

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -lpthread -lm -lrt -ldl $(LDFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS) -lpthread -lm

	chmod a+x $(TARGET)
	@rm *~ -f
	cp fotawin /tftpboot/
clean:
	rm -f $(TARGET) $(OBJS)




















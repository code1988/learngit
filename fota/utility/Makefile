
#export CROSS:=arm-linux-
export CROSS:=
export CC:=$(CROSS)gcc
export AR:=$(CROSS)ar

DVRPATH = ./..

LIBPATH = $(DVRPATH)/lib

CFLAGS = -Wall -O -g -DLINUX

INC_PATH = -I$(DVRPATH)/include


SRC = datetime.c thread.c ipaddr.c mutex.c semaphore.c xbuffer.c
LIBOBJ = $(patsubst %.c, %.o, $(patsubst %.cpp, %.o, $(SRC)))

LIB = $(LIBPATH)/utility.a

all: $(LIB)

%.o: %.c
	$(CC) $(CFLAGS) $(INC_PATH) -c $< -o $@

$(LIB): $(LIBOBJ)
	$(AR) -rcs $@ $^

clean:
	rm -f $(LIB) $(LIBOBJ)




















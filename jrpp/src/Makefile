
include ./common.mk

INCLUDES = -I. -I./jrpplib -I$(STAGING_DIR)/usr/include/jssdk -I$(STAGING_DIR)/usr/include/libjwsconvert
GCCFLAGS = $(CFLAGS) $(INCLUDES) -D__LINUX__ -DJRPP_DBG=1 -DG2F8

CFILES_JRPPD = log.c utils.c libnetlink.c epoll_loop.c brmon.c brstate.c packet.c ctl_socket.c jrpp_if.c jrppd.c 
OFILES_JRPPD = $(CFILES_JRPPD:.c=.o)
CFILES_CTL = utils.c ctl_client.c ctl_main.c log.c ctl_wrap.c
OFILES_CTL = $(CFILES_CTL:.c=.o)

.SUFFIXES: .c .o
.c.o:
	$(CC) $(GCCFLAGS) -c $<  -o $@

all: jrppd jrppctl

jrpplib:
	make -C jrpplib libjrpp.a

.PHONY: jrpplib

jrppd: $(OFILES_JRPPD) jrpplib
	$(CC) $(GCCFLAGS) -o $@ $(OFILES_JRPPD) -L ./jrpplib -ljrpp -ljssdk_us -lubox -ljwsconvert -lubox -lubus -lblobmsg_json

jrppctl: $(OFILES_CTL)
	$(CC) $(GCCFLAGS) -o $@ $(OFILES_CTL)

clean:
	rm -f *.o jrppd jrppctl
	make -C jrpplib clean

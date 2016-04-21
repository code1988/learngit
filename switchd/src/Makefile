
all: switchd

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

SW_LIBS = -lubox -lubus -lblobmsg_json

SW_OBJS = port_config_table.o rate_limite_table.o port_mirror.o

OBJS = utils.o switchd.o ubus.o sysinfo.o signal.o $(SW_OBJS)

switchd: $(OBJS)
	$(CC) -o $@ $(OBJS) $(SW_LIBS)

.PHONY:clean
clean:
	rm -f *.o switchd

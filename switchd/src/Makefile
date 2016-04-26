SRC = $(notdir $(wildcard *.c))
OBJS = $(patsubst %c,%o,$(SRC))
LIBS = -lubox -lubus -lblobmsg_json -ljssdk

all: switchd

switchd: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LIBS)

.PHONY:clean
clean:
	rm -f *.o switchd

.PHONY: all clean cleanall

OBJS=logger.o simulation.o dining-room.o philosopher.o waiter.o # if necessary add new modules
CFLAGS=-Wall -ggdb -pthread	# if necessary add new options
LDFLAGS='-lpthread'
SYMBOLS=-DASCII_SYMBOLS     # alternatives are: -DUTF8_SYMBOLS1 or -DASCII_SYMBOLS

all: simulation

simulation: $(OBJS)
	gcc $(SYMBOLS) $(CFLAGS) $(OBJS) -o simulation

%.o: %.c %.h
	gcc $(SYMBOLS) $(CFLAGS) -c $<

clean:
	rm -fv $(OBJS)

cleanall: clean
	rm -fv simulation

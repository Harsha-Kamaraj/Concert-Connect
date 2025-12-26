CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -O2

OBJS=main.o utils.o users.o events.o bookings.o

all: concert_booking

concert_booking: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

main.o: main.c utils.h users.h events.h bookings.h
utils.o: utils.c utils.h
users.o: users.c users.h utils.h
events.o: events.c events.h utils.h
bookings.o: bookings.c bookings.h users.h events.h utils.h

clean:
	rm -f $(OBJS) concert_booking
# ==== Makefile ====

CC = gcc
CFLAGS = -Wall -pthread
OBJS = main.o consumer.o producer.o globals.o

TARGET = stream_sync

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

main.o: main.c consumer.h producer.h globals.h
	$(CC) $(CFLAGS) -c main.c

consumer.o: consumer.c consumer.h globals.h
	$(CC) $(CFLAGS) -c consumer.c

producer.o: producer.c producer.h globals.h
	$(CC) $(CFLAGS) -c producer.c

globals.o: globals.c globals.h producer.h
	$(CC) $(CFLAGS) -c globals.c

clean:
	rm -f *.o $(TARGET)
	rm -f /tmp/consumer_fifo_*

.PHONY: all clean


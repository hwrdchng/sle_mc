CC=gcc
CFLAGS=-Wall -O3 -fopenmp
LFLAGS=-lm
TARGET=sle_mc
SOURCES:=$(wildcard ./src/*.c)

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) $(LFLAGS) -o $(TARGET)

.PHONY : clean
clean:
	rm -f $(TARGET) *.csv

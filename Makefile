CC = gcc
CFLAGS = -Wall -Wextra -O2 `pkg-config --cflags ncurses`
LDFLAGS = `pkg-config --libs ncurses`
TARGET = typeover
SRC = typeover.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

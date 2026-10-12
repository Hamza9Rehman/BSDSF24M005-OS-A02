CC = gcc
CFLAGS = -Wall -g

TARGET = bin/ls
SRC = src/ls-v1.0.0.c
OBJ = obj/ls-v1.0.0.o

all: dirs $(TARGET)

dirs:
	mkdir -p bin obj

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

clean:
	rm -f bin/* obj/*
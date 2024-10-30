CC = gcc
TARGET = redir
SRC = redir.c
FORMAT_FILES = redir.c

all: format $(TARGET)

format:
	clang-format -i $(FORMAT_FILES)

$(TARGET): $(SRC)
	$(CC) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
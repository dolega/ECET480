ROOT	:= ../../compiler
SOURCE	:= $(ROOT)/lexer/main.cc $(ROOT)/lexer/lexer.cc
CC	:= g++
FLAGS	:= -O3 -std=c++17 -w 
FLAGS	+= -I $(ROOT)
TARGET	:= lexer

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(FLAGS) $(SOURCE) -o $(TARGET)

clean:
	rm -f $(TARGET)

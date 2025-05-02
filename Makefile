# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -g
LDFLAGS = -lm

# Source files
FLEX_SRC = scanner.l
BISON_SRC = parser.y
C_SRC = ast.c schema.c csv.c util.c main.c

# Generated files
FLEX_C = lex.yy.c
BISON_C = parser.tab.c
BISON_H = parser.tab.h

# Object files
OBJS = $(FLEX_C:.c=.o) $(BISON_C:.c=.o) $(C_SRC:.c=.o)

# Target executable
TARGET = json2relcsv

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(FLEX_C): $(FLEX_SRC) $(BISON_H)
	flex $<

$(BISON_C) $(BISON_H): $(BISON_SRC)
	bison -d $<

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(FLEX_C) $(BISON_C) $(BISON_H)
	rm -f *.csv
	rm -f *.o
CC = gcc
CFLAGS =-Wall -pedantic 
SRC=src

EXEC = rash
SOURCES = $(wildcard $(SRC)/*.c)
#OBJECTS = $(SOURCES:$(SRC)/.c=$(OBJ)/.o)
HEADERS = $(wildcard $(SRC)/*.h)

all: $(EXEC) 

$(EXEC): $(SOURCES)
	$(CC) $^ -o $@ -lreadline $(CFLAGS)

clean:
	rm -f $(EXEC) $(OBJECTS)

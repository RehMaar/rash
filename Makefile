CC=gcc
CFLAGS=-Wall
SRC=src
OBJ=obj
OBJF=main.o,command.o,parse.o,environ.o

all: rash

rash: $(SRC)/main.o $(OBJ)/parse.o $(OBJ)/command.o 
	$(CC) -o $@ $? $(CFLAGS) -lreadline

$(OBJ)/main.o: $(SRC)/main.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/parse.o: $(SRC)/parse.c
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ)/command.o: $(SRC)/command.c
	$(CC) -c -o $@ $< $(CFLAGS)  
$(OBJ)/environ.o: $(SRC)/environ.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(OBJ)/{$(OBJF)}

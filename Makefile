# Declaration of variables
CC = gcc
CC_FLAGS = -Wall -pedantic 
SRC=src
OBJ=obj
# File names
EXEC = rash
SOURCES = $(wildcard $(SRC)/*.c)
OBJECTS = $(SOURCES:$(SRC).c=$(OBJ)/.o)

# Main target
$(EXEC): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXEC) -lreadline

# To obtain object files
%.o: %.cpp
	$(CC) -c $(CC_FLAGS) $< -o $@

# To remove generated files
clean:
	rm -f $(EXEC) $(OBJECTS)

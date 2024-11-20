# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Output library name
LIBRARY = libmiddleware.a

# Source files
SRC = middleware.c

# Object files
OBJ = middleware.o

# Rule to create the static library
$(LIBRARY): $(OBJ)
	ar rcs $(LIBRARY) $(OBJ)

# Rule to compile prueba.c into prueba.o
$(OBJ): $(SRC)
	$(CC) $(CFLAGS) -c $(SRC)

# Clean rule to remove generated files
clean:
	rm -f $(OBJ) $(LIBRARY)

# Phony targets (not actual files)
.PHONY: clean


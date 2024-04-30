# vim:tabstop=2

# Compilation variables
CC=gcc
CFLAGS=-lpthread -lrt -Wall

# Build variables
OUT_DIR=./out
PROGRAM_1=$(OUT_DIR)/program-1
PROGRAM_2=$(OUT_DIR)/program-2

.PHONY: default clean

# Build both programs by default
default: $(PROGRAM_1) $(PROGRAM_2)

# Build `program_1`
$(PROGRAM_1): program_1.c
	$(CC) $< -o $(PROGRAM_1) $(CFLAGS)

# Build `program_2`
$(PROGRAM_2): program_2.c
	$(CC) $< -o $(PROGRAM_2) $(CFLAGS)

# Clean build artefacts
clean:
	rm -f $(PROGRAM_1) $(PROGRAM_2)

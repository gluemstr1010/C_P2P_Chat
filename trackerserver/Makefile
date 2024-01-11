GCC_FLAGS=-g -Wall -Wextra

TRCK_SRC_DIR=src/
TRCK_OBJ_DIR = trackerobj/
TRCK_HDR_DIR=trackerlib/

TRCK_SRC_FILES=$(wildcard $(TRCK_SRC_DIR)/*.c)
TRCK_OBJ_FILES=$(patsubst %, $(TRCK_OBJ_DIR)/%,$(notdir $(TRCK_SRC_FILES:.c=.o)))


BINARY=trackerserver

all: $(BINARY)

$(BINARY): $(TRCK_OBJ_FILES)
	gcc $(GCC_FLAGS) $(TRCK_OBJ_FILES) -o $(BINARY)

$(TRCK_OBJ_DIR)/%.o: $(TRCK_SRC_DIR)/%.c
	gcc $(GCC_FLAGS) -c $^ -I$(TRCK_HDR_DIR) -o $@

$(TRCK_OBJ_DIR):
	mkdir $@

clean:
	rm -rf $(BINARY) $(TRCK_OBJ_DIR)
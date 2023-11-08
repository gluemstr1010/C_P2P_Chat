GCC_FLAGS=-g -Wall

CL_SRC_DIR=src/
CL_OBJ_DIR = clientobj/
CL_HDR_DIR=clientlib/

CL_SRC_FILES=$(wildcard $(CL_SRC_DIR)/*.c)
CL_OBJ_FILES=$(patsubst %, $(CL_OBJ_DIR)/%,$(notdir $(CL_SRC_FILES:.c=.o)))


BINARY=client

all: $(BINARY)

$(BINARY): $(CL_OBJ_FILES)
	gcc $(GCC_FLAGS) $(CL_OBJ_FILES) -o $(BINARY)

$(CL_OBJ_DIR)/%.o: $(CL_SRC_DIR)/%.c
	gcc $(GCC_FLAGS) -c $^ -I$(CL_HDR_DIR) -o $@

$(CL_OBJ_DIR):
	mkdir -p $(CL_OBJ_DIR)

clean:
	rm -rf $(BINARY) $(CL_OBJ_DIR)
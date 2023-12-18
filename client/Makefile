GCC_FLAGS=-g -Wall -Wshadow -Wextra
LIBS=-lcurl


CL_SRC_DIR=src/
CL_OBJ_DIR = clientobj/
CL_HDR_DIR=clientlib/

CL_SRC_FILES=$(wildcard $(CL_SRC_DIR)/*.c)
CL_OBJ_FILES=$(patsubst %, $(CL_OBJ_DIR)/%,$(notdir $(CL_SRC_FILES:.c=.o)))



BINARY=client

all: $(BINARY)

$(CL_OBJ_DIR):
	@mkdir -p $(@D)

$(BINARY): $(CL_OBJ_FILES)
	gcc $(GCC_FLAGS) $(CL_OBJ_FILES) -o $(BINARY) $(LIBS)

$(CL_OBJ_DIR)/%.o: $(CL_SRC_DIR)/%.c
	gcc $(GCC_FLAGS) -c $^ -I$(CL_HDR_DIR) -o $@

clean:
	@rm -rf $(BINARY) $(CL_OBJ_FILES)
GCC_FLAGS=-g -Wall -Wshadow -Wextra

CL_SRC_DIR=src/
CL_OBJ_DIR = clientobj/
CL_HDR_DIR=clientlib/
ENC_HDR_DIR=../enc/inc
ENC_SRC_DIR=../enc/rsa

CL_SRC_FILES=$(wildcard $(CL_SRC_DIR)/*.c)
ENC_SRC_FILES=$(wildcard $(ENC_SRC_DIR)/*.c)

CL_OBJ_FILES=$(patsubst %, $(CL_OBJ_DIR)/%,$(notdir $(CL_SRC_FILES:.c=.o)))
ENC_OBJ_FILES = $(patsubst $(ENC_SRC_DIR)/%.c,$(CL_OBJ_DIR)/%.o,$(ENC_SRC_FILES))


BINARY=client

all: $(BINARY)

$(BINARY): $(CL_OBJ_FILES) $(ENC_OBJ_FILES)
	gcc $(GCC_FLAGS) $^ -o $@ -lm -lgmp

$(CL_OBJ_DIR):
	mkdir -p $@

$(CL_OBJ_DIR)/%.o: $(CL_SRC_DIR)/%.c | $(CL_OBJ_DIR)
	gcc $(GCC_FLAGS) -c $< -I$(CL_HDR_DIR) -o $@

$(CL_OBJ_DIR)/%.o: $(ENC_SRC_DIR)/%.c | $(CL_OBJ_DIR)
	gcc $(GCC_FLAGS) -c $< -I$(ENC_HDR_DIR) -o $@

clean:
	rm -rf $(BINARY) $(CL_OBJ_DIR)
GCC_FLAGS = -g -Og -Wall -Wextra

TRCK_SRC_DIR = src
TRCK_OBJ_DIR = trackerobj
TRCK_HDR_DIR = trackerlib

ENC_HDR_DIR = ../enc/inc
ENC_SRC_DIR = ../enc/rsa

SER_HDR_DIR = ../serialization/inc
SER_SRC_DIR = ../serialization/src


TRCK_SRC_FILES = $(wildcard $(TRCK_SRC_DIR)/*.c)
ENC_SRC_FILES = $(wildcard $(ENC_SRC_DIR)/*.c)
SER_SRC_FILES = $(wildcard $(SER_SRC_DIR)/*.c)


TRCK_OBJ_FILES = $(patsubst $(TRCK_SRC_DIR)/%.c,$(TRCK_OBJ_DIR)/%.o,$(TRCK_SRC_FILES))
ENC_OBJ_FILES = $(patsubst $(ENC_SRC_DIR)/%.c,$(TRCK_OBJ_DIR)/%.o,$(ENC_SRC_FILES))
SER_OBJ_FILES = $(patsubst $(SER_SRC_DIR)/%.c,$(TRCK_OBJ_DIR)/%.o,$(SER_SRC_FILES))

BINARY = trackerserver


all: $(BINARY)

$(BINARY): $(TRCK_OBJ_FILES) $(ENC_OBJ_FILES) $(SER_OBJ_FILES)
	gcc $(GCC_FLAGS) $^ -o $@ -lm -lgmp -lssl -lcrypto -D_XOPEN_SOURCE=500

$(TRCK_OBJ_DIR):
	mkdir -p $@

$(TRCK_OBJ_DIR)/%.o: $(TRCK_SRC_DIR)/%.c | $(TRCK_OBJ_DIR)
	gcc $(GCC_FLAGS) -c $< -I$(TRCK_HDR_DIR) -o $@


$(TRCK_OBJ_DIR)/%.o: $(ENC_SRC_DIR)/%.c | $(TRCK_OBJ_DIR)
	gcc $(GCC_FLAGS) -c $< -I$(ENC_HDR_DIR) -o $@

$(TRCK_OBJ_DIR)/%.o: $(SER_SRC_DIR)/%.c | $(TRCK_OBJ_DIR)
	gcc $(GCC_FLAGS) -c $< -I$(ENC_HDR_DIR) -o $@

clean:
	rm -rf $(BINARY) $(TRCK_OBJ_DIR)

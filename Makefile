BUILD_DIR = build
SRC_DIR = src

all: blast

clean:
	rm -rf $(BUILD_DIR)/*.o

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(@D)
	gcc -Isrc -c $< -o $@ -Os -std=c11 $(CFLAGS)

C_FILES = $(wildcard $(SRC_DIR)/*.c)
C_FILES += $(wildcard $(SRC_DIR)/sensors/*.c)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

blast: $(OBJ_FILES)
	gcc -o $(BUILD_DIR)/blast $(OBJ_FILES) -lmraa -std=c11 -Os

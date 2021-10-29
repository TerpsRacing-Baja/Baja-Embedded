BUILD_DIR = build

all: blast

clean:
	rm -rf $(BUILD_DIR)/*.o

$(BUILD_DIR)/%.o: ./%.c
	mkdir -p $(@D)
	gcc -c $< -o $@

C_FILES = $(wildcard ./*.c)
OBJ_FILES = $(C_FILES:./%.c=$(BUILD_DIR)/%.o)

DEP_FILES = $(OBJ_FILES:%.o=%.d)
-include $(DEP_FILES)

blast: $(BUILD_DIR)/blast.o $(BUILD_DIR)/blast_data.o
	gcc -o $(BUILD_DIR)/blast $(BUILD_DIR)/blast.o $(BUILD_DIR)/blast_data.o

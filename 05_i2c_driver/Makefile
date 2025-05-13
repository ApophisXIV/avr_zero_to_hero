MCU_TARGET = atmega328p

TARGET_NAME = main

TARGET_FORMAT = .elf
TARGET = $(BUILD_DIR)$(TARGET_NAME)$(TARGET_FORMAT)

TARGET_PRODUCTION_FORMAT = .hex
TARGET_PRODUCTION = $(BUILD_DIR)$(TARGET_NAME)$(TARGET_PRODUCTION_FORMAT)

EEP_FORMAT = .eep
EEP_TARGET = $(BUILD_DIR)$(TARGET_NAME)$(EEP_FORMAT)

# -------------------------------- Programmer -------------------------------- #
PROGRAMMER_MCU_NAME = m328p
PROGRAMMER = usbtiny
PROGRAMMER_PORT =
PROGRAMMER_EXITSPEC = -E reset

AVRDUDE = avrdude
AVRDUDE_LOAD_SPEED = -B 1
AVR_DUDE_OPTIONS = -p $(PROGRAMMER_MCU_NAME) -c $(PROGRAMMER) $(PROGRAMMER_PORT) $(AVRDUDE_EXITSPEC)

# -------------------------------- Files paths ------------------------------- #
BUILD_DIR = ./build/
SRC_DIR = ./
SRC_FILES = $(shell find $(SRC_DIR) -type f -name '*.c')
OBJ_FILES = $(patsubst $(SRC_DIR)%.c,$(BUILD_DIR)%.o,$(SRC_FILES))

FILTER_DIRS = ./Utils 

# Filter out the directories that are not in the FILTER_DIRS list
SRC_FILES := $(filter-out $(foreach dir,$(FILTER_DIRS),$(dir)%.c),$(SRC_FILES))
OBJ_FILES := $(filter-out $(foreach dir,$(FILTER_DIRS),$(dir)%.o),$(OBJ_FILES))

# --------------------------------- Compiler --------------------------------- #
CC = avr-gcc
CSTD = -std=c99
CWARN = -Wall -Wstrict-prototypes -Wall -Wextra -Wno-unused-parameter -Wshadow 
CDEFS =
COPTMLEVEL= 2
COPTM = -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums 
# CFLAGS = $(CWARN) $(CDEFS) $(COPTM) $(CSTD) -O$(COPTMLEVEL) 
CFLAGS = $(CWARN) $(CDEFS) $(CSTD) -O$(COPTMLEVEL) 

# ----------------------------------- Utils ---------------------------------- #
AVR_OBJCPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size

MKDIR = mkdir -p
RM = rm -rf

NULLDEV = 2>/dev/null
ifeq ($(OS),Windows_NT)
  NULLDEV = 2>NUL
endif

# Error messages and decorations
H_BAR_RED = @echo '\033[1;31m=====================================================================================\033[0m';
H_BAR_GREEN = @echo '\033[1;32m=====================================================================================\033[0m';
H_BAR_YELLOW = @echo '\033[1;33m=====================================================================================\033[0m';
H_BAR_BLUE = @echo '\033[1;34m=====================================================================================\033[0m';
H_BAR_PINK = @echo '\033[1;35m=====================================================================================\033[0m';
H_BAR_CYAN = @echo '\033[1;36m=====================================================================================\033[0m';
H_BAR_WHITE = @echo '\033[1;37m=====================================================================================\033[0m';

define print_clean_msg
	$(H_BAR_RED)
	@echo '\033[1;45m --> \033[0;1;33m Cleaning $(BUILD_DIR) directory... \033[0m';
	$(H_BAR_RED)
endef

define compile
	@$(H_BAR_YELLOW)
	@echo -e '\033[1;41m --> \033[0;1;34m Compiling: \033[0m \033[1;33m $(shell basename $(1)) \033[0m\033[0m'
	@mkdir -p $(BUILD_DIR)$(dir $<)
	$(CC) $(CFLAGS) -mmcu=$(MCU_TARGET) -c $(1) -o $(2)
	@$(H_BAR_YELLOW)
endef

define link
	$(H_BAR_CYAN)
	@echo '\033[1;41m --> \033[0;1;34m Linking: \033[0m \033[1;36m $(shell basename $(1)) \033[0m\033[0m';
	@$(H_BAR_CYAN)
	$(CC) $(CFLAGS) -mmcu=$(MCU_TARGET) -o $(1) $(2) $(LDFLAGS)
	@$(H_BAR_CYAN)
endef

# -------------------------------- Common libs ------------------------------- #
LIB_PRINTF_MIN =  -Wl,-u,vfprintf -lprintf_min
LIB_PRINTF_FLOAT = -Wl,-u,vfprintf -lprintf_flt
LIB_SCANF_MIN =  -Wl,-u,vfscanf -lscanf_min
LIB_SCANF_FLOAT = -Wl,-u,vfscanf -lscanf_flt
LIB_MATH = -lm

LDFLAGS = $(LIB_PRINTF_FLOAT) $(LIB_MATH)

# ---------------------------------- Recipes --------------------------------- #
all: $(TARGET) $(EEP_TARGET) mem_usage

$(TARGET): $(OBJ_FILES)
	$(call link,$@,$^)
	@$(AVR_OBJCPY) -O ihex $@ $(TARGET_PRODUCTION)

$(BUILD_DIR)%.o: $(SRC_DIR)%.c
	$(call compile,$<,$@)

$(BUILD_DIR)fuses.hex: $(TARGET)
	@echo "looking for fuses in $(TARGET)"
	@$(AVR_OBJCPY) -O binary -j .fuse $< $@
	@echo "Creating fuses file fuses.hex"

fuses: $(TARGET) $(BUILD_DIR)fuses.hex
	$(H_BAR_PINK)
	@echo '\033[1;41m --> \033[0;1;34m Flashing fuses: \033[0m \033[1;35m $(shell basename $()) \033[0m\033[0m';
	$(H_BAR_PINK)
	@FUSES_HEX_CONTENT=$$(xxd -ps -c 1 $(BUILD_DIR)fuses.hex); \
	LFUSE=$$(echo $$FUSES_HEX_CONTENT | cut -d' ' -f1); \
	HFUSE=$$(echo $$FUSES_HEX_CONTENT | cut -d' ' -f2); \
	EFUSE=$$(echo $$FUSES_HEX_CONTENT | cut -d' ' -f3); \
	echo LFUSE:$$LFUSE\ HFUSE:$$HFUSE\ EFUSE:$$EFUSE; echo; \
	$(AVRDUDE) $(AVR_DUDE_OPTIONS) \
	-U lfuse:w:0x$$LFUSE:m \
	-U hfuse:w:0x$$HFUSE:m \
	-U efuse:w:0x$$EFUSE:m
	$(H_BAR_PINK)

$(EEP_TARGET): $(TARGET)
	$(H_BAR_CYAN)
	@echo '\033[1;41m --> \033[0;1;37m EEPROM: \033[0m \033[1;36m $(shell basename $(EEP_TARGET)) \033[0m\033[0m';
	@$(H_BAR_CYAN)
	$(AVR_OBJCPY) -O ihex -j .eeprom --change-section-lma .eeprom=0 $< $@
	@$(H_BAR_CYAN)

eeprom: $(EEP_TARGET)
	$(H_BAR_GREEN)
	@echo '\033[1;41m --> \033[0;1;37m Flashing EEPROM to MCU: \033[0m \033[1;32m  $(TARGET_NAME)$(EEP_FORMAT) \033[0m\033[0m';
	$(H_BAR_GREEN)
	$(AVRDUDE) $(AVR_DUDE_OPTIONS) -U eeprom:w:$(EEP_TARGET):i

flash: $(TARGET) mem_usage
	$(H_BAR_GREEN)
	@echo '\033[1;41m --> \033[0;1;37m Flashing program to MCU: \033[0m \033[1;32m  $(TARGET_NAME)$(TARGET_FORMAT) \033[0m\033[0m';
	$(H_BAR_GREEN)
	$(AVRDUDE) $(AVR_DUDE_OPTIONS) $(AVRDUDE_LOAD_SPEED) -U flash:w:$(TARGET)
	$(H_BAR_GREEN)

mem_usage: $(TARGET)
	$(H_BAR_BLUE)
	@echo '\033[1;41m --> \033[0;1;37m Memory usage: \033[0m \033[1;32m $(shell basename $(TARGET)) \033[0m\033[0m';
	$(H_BAR_BLUE)
	@$(SIZE) --format=avr --mcu=$(MCU_TARGET) $(TARGET)
	$(H_BAR_BLUE)

clean:
	$(call print_clean_msg)
	@$(RM) $(BUILD_DIR) $(TARGET)

debug:
	@echo -e $(SRC_FILES)
	@echo -e $(OBJ_FILES)

.PHONY: all clean flash fuses debug
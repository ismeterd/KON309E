TARGET = experiment1_p2
MAP_FILE=$(TARGET).map
HEX_FILE=$(TARGET).hex
ELF_FILE=$(TARGET).elf
BIN_FILE=$(TARGET).bin


# Build directory
BUILD_DIR = build

# Processor is: LPC824M201JHI33

# C sources
C_SOURCES =  experiment1_p2.c
C_SOURCES += init.c



# ASM sources
# None


#Tools:
CC = arm-none-eabi-gcc
AS = arm-none-eabi-gcc -x assembler-with-cpp
CP = arm-none-eabi-objcopy
LD = arm-none-eabi-ld
SZ = arm-none-eabi-size

HEX_FLAGS = -O ihex
BIN_FLAGS =  -O binary -S 

C_INCLUDES  = -I.


C_FLAGS  = -c -mcpu=cortex-m0 -mthumb -g
# The following line adds dependency files:
C_FLAGS += -MMD -MP  -MF"$(@:%.o=%.d)"
# The following line adds list files:
C_FLAGS += -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))
# Add assembly listing of each compiled file to the build dir:
##CFLAGS += -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst))
# -Wa: pass options to assembler.
# -alms specifies:
# -a: turn on listings
# -d: omit debugging directives; n: omit forms processing
# -h: include high-level source
# -l: include assembly


# LD flags, linker script:
LDSCRIPT = lpc824_linker_script.ld

#LDFLAGS += -T$(LDSCRIPT) -Map $(BUILD_DIR)/$(MAP_FILE)
LDFLAGS += -T $(LDSCRIPT) --cref -Map $(BUILD_DIR)/$(MAP_FILE)


# default action: build all
all: $(BUILD_DIR)/$(ELF_FILE) $(BUILD_DIR)/$(HEX_FILE) $(BUILD_DIR)/$(BIN_FILE)


#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))

# Objects from c sources:
$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) $(C_FLAGS) $< -o $@

# Objects from assembly sources: CAUTION: Check 'S' or 's' extension.
$(BUILD_DIR)/%.o: %.s Makefile #| $(BUILD_DIR)
	$(AS) -c $(C_FLAGS) $< -o $@

$(BUILD_DIR)/$(ELF_FILE): $(OBJECTS) Makefile
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@
#	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(CP) $(HEX_FLAGS) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(CP) $(BIN_FLAGS) $< $@	

$(BUILD_DIR):
	mkdir $@

sflash: $(BUILD_DIR)/$(ELF_FILE)
	@echo Make sure the device is in Boot 0 mode \(Press SW2/ISP switch while coming out of reset.\)
	lpcprog -d /dev/ttyUSB0 -c flash $(BUILD_DIR)/$(BIN_FILE)

21flash: $(BUILD_DIR)/$(HEX_FILE)
	lpc21isp -control $(BUILD_DIR)/$(HEX_FILE) /dev/ttyUSB0 115200 12000

clean:
	rm -f $(OBJECTS) $(OBJECTS:.o=.d) $(OBJECTS:.o=.lst) $(BUILD_DIR)/$(ELF_FILE) $(BUILD_DIR)/$(BIN_FILE) $(BUILD_DIR)/$(MAP_FILE)

# Dependencies
-include $(wildcard $(BUILD_DIR)/*.d)




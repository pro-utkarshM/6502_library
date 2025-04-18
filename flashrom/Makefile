# Name of the ROM binary file
ROM_BIN = rom.bin

# Python scripts
MAKE_ROM = make_rom.py
FLASH_ROM = flash.py

# Default target
all: flash

# Rule to create the ROM binary
$(ROM_BIN): $(MAKE_ROM)
	@echo "[+] Generating ROM binary..."
	python3 $(MAKE_ROM)

# Rule to flash the ROM
flash: $(ROM_BIN)
	@echo "[+] Flashing ROM to AT28C256..."
	sudo python3 $(FLASH_ROM)

# Clean up the ROM file
clean:
	rm -f $(ROM_BIN)
	@echo "[+] Cleaned up generated files."

.PHONY: all flash clean

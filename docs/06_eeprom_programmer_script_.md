# Chapter 6: EEPROM Programmer Script

Welcome to the final chapter focused on the EEPROM! In the [previous chapter](05_rom_image_generation_script_.md), we explored the **ROM Image Generation Script** (`make_rom.py`), which creates the `rom.bin` file – the digital blueprint containing your program's bytes and data, correctly formatted for the 32KB EEPROM.

Now that we have the blueprint, how do we get it from the computer and onto the physical AT28C256 chip? This is the job of the **EEPROM Programmer Script**, the main Python program (`flash.py`) that runs on your Raspberry Pi.

### What Does the Programmer Script Do?

Imagine you have a complex pattern (your `rom.bin` file) and you need to transfer it precisely onto a surface (the EEPROM chip). The EEPROM programmer script is the specialized machine that performs this transfer.

Its core purpose is to:

1.  **Read the Blueprint:** Open the `rom.bin` file and read the sequence of bytes that need to be programmed.
2.  **Operate the Hardware:** Use the Raspberry Pi's GPIO pins to control the EEPROM chip according to the wiring we set up in [Chapter 1: EEPROM Hardware Connections](01_eeprom_hardware_connections_.md).
3.  **Write Byte by Byte:** Go through each byte in the `rom.bin` file, determine its corresponding address, and perform the precise electrical sequence (using address, data, and control signals, as learned in [Chapter 2: Address and Data Interface](02_address_and_data_interface_.md) and [Chapter 3: EEPROM Control Signals](03_eeprom_control_signals_.md)) to write that byte into the correct location on the EEPROM.
4.  **Manage Timing:** Ensure the operations happen with the exact timing required by the EEPROM chip's specifications, especially the crucial wait time after each byte is written.

In short, `flash.py` is the bridge. It takes the abstract digital data from `rom.bin` and translates it into the physical electrical pulses needed to change the memory cells inside the EEPROM chip.

### Running the Programmer Script

You'll typically run this script on your Raspberry Pi *after* you've connected the EEPROM hardware as shown in [Chapter 1](01_eeprom_hardware_connections_.md) and generated the `rom.bin` file using `make_rom.py` ([Chapter 5](05_rom_image_generation_script_.md)).

The simplest way to run it is using the project's `Makefile`:

```bash
make flash
```

Let's quickly see what `make flash` does by looking at the `Makefile` snippet:

```makefile
# flashrom/Makefile
# ... other parts ...

# Name of the ROM binary file
ROM_BIN = rom.bin

# Python scripts
MAKE_ROM = make_rom.py
FLASH_ROM = flash.py

# Rule to flash the ROM
flash: $(ROM_BIN)
	@echo "[+] Flashing ROM to AT28C256..."
	sudo python3 $(FLASH_ROM)

# ... other parts ...
```

When you run `make flash`, the `Makefile` first checks the dependency `$(ROM_BIN)`. This means it will run the `make_rom.py` script first if needed (as we saw in [Chapter 5](05_rom_image_generation_script_.md)) to ensure `rom.bin` is up-to-date. Once `rom.bin` is ready, `make` executes the command `sudo python3 $(FLASH_ROM)`, which is `sudo python3 flash.py`.

You need `sudo` because accessing and controlling the Raspberry Pi's GPIO pins directly from a user-space script typically requires root privileges for safety and hardware control.

### Inside the `flash.py` Script

Let's break down the key parts of the `flash.py` script to see how it performs its task. We'll see how it uses the concepts from the previous chapters.

#### Setting Up the GPIO Pins

First, the script needs to configure the Raspberry Pi's GPIO pins.

```python
# flashrom/flash.py
import RPi.GPIO as GPIO
import time

# Pin definitions - Matches wiring from Chapter 1
ADDR_PINS = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16] # A0-A14
DATA_PINS = [17, 18, 19, 20, 21, 22, 23, 24]                     # D0-D7
CE = 25 # /CE (Chip Enable)
OE = 26 # /OE (Output Enable)
WE = 27 # /WE (Write Enable)

# Delay needed after a write pulse (check EEPROM datasheet)
WRITE_DELAY = 0.001 # 1 millisecond

def setup():
    # Tell RPi.GPIO to use the BCM numbering scheme
    GPIO.setmode(GPIO.BCM)

    # Set all relevant pins as outputs initially
    for pin in ADDR_PINS + DATA_PINS + [CE, OE, WE]:
        GPIO.setup(pin, GPIO.OUT)

    # Set control pins to inactive state initially
    GPIO.output(CE, 1) # /CE HIGH (chip disabled)
    GPIO.output(OE, 1) # /OE HIGH (outputs disabled)
    GPIO.output(WE, 1) # /WE HIGH (write disabled)

# ... rest of the script ...
```

This `setup()` function is called first. It initializes the RPi.GPIO library, tells it which pins we will be using, and configures all the address, data, and control pins as *outputs*. They are all set high initially to ensure the EEPROM is inactive until the script is ready. This uses the pin definitions based on our wiring from [Chapter 1](01_eeprom_hardware_connections_.md).

#### Setting the Address

As covered in [Chapter 2](02_address_and_data_interface_.md), we need to set the address lines to select a specific location in the EEPROM.

```python
# flashrom/flash.py
# ... pin definitions and setup() ...

def set_address(addr):
    # Loop through each address pin (A0 through A14)
    # 'i' is the bit position (0 to 14), 'pin' is the corresponding GPIO number
    for i, pin in enumerate(ADDR_PINS):
        # Extract the i-th bit from the address number 'addr'
        bit_value = (addr >> i) & 1
        # Set the GPIO pin to HIGH (1) or LOW (0)
        GPIO.output(pin, bit_value)

# ... rest of the script ...
```

The `set_address()` function takes an address number (like `0`, `1`, `5`, etc.) and translates it into the correct combination of HIGH/LOW signals on the GPIO pins connected to the EEPROM's address lines.

#### Setting the Data

Similarly, before writing, the script needs to put the byte to be written onto the data lines, as discussed in [Chapter 2](02_address_and_data_interface_.md).

```python
# flashrom/flash.py
# ... set_address() and other functions ...

def set_data(data):
    # Loop through each data pin (I/O0 through I/O7)
    # 'i' is the bit position (0 to 7), 'pin' is the corresponding GPIO number
    for i, pin in enumerate(DATA_PINS):
        # Extract the i-th bit from the data byte 'data'
        bit_value = (data >> i) & 1
        # Set the GPIO pin to HIGH (1) or LOW (0)
        GPIO.output(pin, bit_value)

# ... rest of the script ...
```

The `set_data()` function takes a byte value (0-255) and sets the GPIO pins connected to the EEPROM's data lines (I/O0-I/O7) to represent that byte in binary.

#### Writing a Single Byte (The Core Operation)

This is where the control signals from [Chapter 3](03_eeprom_control_signals_.md) come into play. The `write_byte` function orchestrates the precise sequence of signals needed to write one byte.

```python
# flashrom/flash.py
# ... set_data() and other functions ...

def write_byte(addr, data):
    # Step 1: Set the address lines (using function above)
    set_address(addr)
    # Step 2: Set the data lines with the byte to write (using function above)
    set_data(data)

    # Step 3: Activate the Chip (/CE LOW)
    GPIO.output(CE, 0) # GPIO 25 LOW

    # Step 4: Ensure Output is Disabled (/OE HIGH - crucial for writing)
    GPIO.output(OE, 1) # GPIO 26 HIGH

    # Step 5: Trigger the Write (/WE LOW)
    GPIO.output(WE, 0) # GPIO 27 LOW

    # Step 6: Wait for the required WE pulse width (very short)
    time.sleep(0.00001) # 10 microseconds - example, check datasheet

    # Step 7: End the Write Trigger (/WE HIGH)
    GPIO.output(WE, 1) # GPIO 27 HIGH

    # Step 8: Deactivate the Chip (/CE HIGH)
    GPIO.output(CE, 1) # GPIO 25 HIGH

    # Step 9: Wait for the internal write cycle to complete (milliseconds)
    time.sleep(WRITE_DELAY) # Use the defined delay (e.g., 1ms)

# ... rest of the script ...
```

This `write_byte` function brings together the concepts from Chapters 2 and 3. It first sets the address and data lines, then carefully toggles the `/CE`, `/OE`, and `/WE` control signals in the correct order and with the necessary pauses (`time.sleep`) to trigger the EEPROM's internal write process. The `WRITE_DELAY` is particularly important; the script *must* wait for the EEPROM to finish saving the byte before attempting to write the next one.

#### Flashing the Entire ROM Image

Finally, the `flash_rom` function reads the `rom.bin` file and uses the `write_byte` function to program each byte.

```python
# flashrom/flash.py
# ... write_byte() and other functions ...

def flash_rom(filename):
    # Open the specified binary file for reading
    with open(filename, "rb") as f:
        # Read the entire content into a byte sequence
        data = f.read()
        print(f"[+] Flashing {len(data)} bytes")

        # Loop through each byte in the 'data' sequence
        # 'i' is the index (which becomes the EEPROM address)
        # 'byte' is the value at that index
        for i, byte in enumerate(data):
            # Call write_byte to program this byte at address 'i'
            write_byte(i, byte)

            # Print occasional progress updates (optional)
            if i % 521 == 0: # Print approx every 512 bytes (prime number used to avoid patterns)
                 print(f"[{i:04X}] → 0x{byte:02X}")

        print("[✓] Done flashing")

# Main execution block
if __name__ == "__main__":
    try:
        setup() # Configure GPIO pins
        flash_rom("rom.bin") # Start the flashing process using rom.bin
    finally:
        cleanup() # Reset GPIO pins gracefully
```

The `flash_rom` function opens `rom.bin` (referencing [Chapter 4](04_rom_image_file_.md)), reads all its contents into the `data` variable. It then uses `enumerate` to loop through `data`. For each byte it finds, `i` is the byte's position (starting from 0), and `byte` is its value. The script simply calls `write_byte(i, byte)`, delegating the actual hardware control for that single byte to the `write_byte` function. This continues until every byte from the file has been written to the EEPROM.

#### Cleanup

The `cleanup()` function is simple but important:

```python
# flashrom/flash.py
# ... flash_rom() and other functions ...

def cleanup():
    # Reset all GPIO settings
    GPIO.cleanup()

# ... main execution block calls cleanup() ...
```

It calls `GPIO.cleanup()`, which resets all the GPIO pins used by the script back to their default state (usually inputs with pull-downs). This is good practice to avoid leaving pins in unexpected states after the script finishes, which could interfere with other programs or hardware. The `try...finally` block ensures `cleanup()` is called even if an error occurs during flashing.


### Conclusion

The EEPROM programmer script (`flash.py`) is the core program that brings together all the concepts we've discussed: the physical connections, the address and data interfaces, and the precise control signals and timing needed for writing. It reads the program blueprint (`rom.bin` file) and, byte by byte, uses the Raspberry Pi's GPIO pins to replicate the necessary electrical signals and delays to store that information reliably onto the EEPROM chip.

Once this script has finished successfully, your AT28C256 EEPROM contains the program and data specified by your `rom.bin` file, ready to be placed into your 6502 computer and executed!

This concludes the chapters specifically focused on the EEPROM and its programming process within the `6502_library` project. You now have a solid understanding of how a program goes from assembly code to a binary file and finally gets written onto the chip.

---

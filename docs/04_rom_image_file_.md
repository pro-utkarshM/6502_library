# Chapter 4: ROM Image File

Welcome back! In the [previous chapters](01_eeprom_hardware_connections_.md), we've focused on the physical and electrical aspects of talking to the AT28C256 EEPROM using your Raspberry Pi. We've covered:

*   [Chapter 1: EEPROM Hardware Connections](01_eeprom_hardware_connections_.md): Wiring up the pins.
*   [Chapter 2: Address and Data Interface](02_address_and_data_interface_.md): How address and data lines select locations and transfer bytes.
*   [Chapter 3: EEPROM Control Signals](03_eeprom_control_signals_.md): The timing and control signals (/CE, /OE, /WE) needed to perform read and write operations.

Now you know *how* the Raspberry Pi can write a single byte to a specific address on the EEPROM. But what about putting an entire program or a large amount of data onto the chip? Your 6502 computer doesn't just need one byte; it needs thousands!

This is where the concept of the **ROM Image File** comes in.

### What is a ROM Image File?

Imagine you have written a small program in 6502 assembly language. This program, once assembled, turns into a sequence of bytes – the machine code instructions that the 6502 processor understands. You also might have some data your program needs, like text messages or numbers, which are also represented as bytes.

All these bytes – the instructions and the data – need to be stored on the EEPROM in a specific order and at specific memory addresses so that the 6502 can find and execute the program.

A **ROM Image File** is simply a digital file on your computer (like `rom.bin` in this project) that contains the *exact sequence* of bytes you want to store on the EEPROM, from address 0 up to the highest address you need.

*   It's a **raw binary file**: It's not a text file; you can't easily read it with a standard text editor. It's just a sequence of 0s and 1s grouped into bytes.
*   It's the **digital blueprint**: It represents the final state of the EEPROM's contents byte-for-byte. The first byte in the file will go to EEPROM address 0, the second byte to address 1, and so on.
*   It includes **everything**: Machine code instructions, data, and any special values the computer needs (like the "reset vector" that tells the 6502 where to start executing code after reset).

Think of it like a stencil or a stamp. The `rom.bin` file is the stencil pattern. The EEPROM programmer script (`flash.py`) is the tool that applies this pattern (the bytes) onto the "surface" of the EEPROM chip, byte by byte, position by position (address by address).

### Why Do We Need This File?

We need the ROM image file because:

1.  **Organization:** It collects all the necessary bytes for your program and data into a single, convenient package.
2.  **Input for Programmer:** The EEPROM programmer script needs a single source of truth telling it *what* to write to the EEPROM. The ROM image file provides this list of bytes.
3.  **Reproducibility:** If you want to program multiple EEPROMs with the same content, you just use the same `rom.bin` file.

The overall process looks something like this:

![how](/assets/how.png)

Your assembly code is processed by an assembler into raw bytes. The `make_rom.py` script (which we'll cover in the next chapter) takes these bytes and arranges them correctly into the `rom.bin` file. Finally, `flash.py` reads `rom.bin` and writes its contents to the EEPROM, which can then be used by your 6502 computer.

### The ROM Image File in the Flashing Process

The `flash.py` script, which is responsible for programming the EEPROM using the methods discussed in Chapters 1-3, gets the data it needs *from* the `rom.bin` file.

Here's how `flash.py` uses the ROM image file:

1.  It specifies the name of the file to read (`rom.bin`).
2.  It opens this file for reading in binary mode.
3.  It reads the entire contents of the file into memory as a sequence of bytes.
4.  It then loops through this sequence of bytes. For each byte:
    *   The position of the byte in the sequence corresponds to the memory **address** on the EEPROM where it should be written.
    *   The value of the byte is the **data** that should be written to that address.
    *   It calls the `write_byte(address, data)` function (which we saw the details of in [Chapter 3: EEPROM Control Signals](03_eeprom_control_signals_.md)) using the position as the address and the byte's value as the data.

### Looking at the Code (Simplified)

Let's look at the relevant part of the `flash.py` script to see how it handles the `rom.bin` file.

First, the `flash_rom` function takes the filename as input:

```python
def flash_rom(filename):
    # Open the specified file in binary read mode ('rb')
    with open(filename, "rb") as f:
        # Read all bytes from the file
        data = f.read()
        print(f"[+] Flashing {len(data)} bytes")

        # ... rest of the function ...
```

This is standard Python file handling. `f.read()` reads the entire content of the file into the `data` variable, which becomes a sequence of bytes. `len(data)` tells us how many bytes are in the file.

Next, the script loops through each byte in the `data` sequence:

```python
def flash_rom(filename):
    with open(filename, "rb") as f:
        data = f.read()
        print(f"[+] Flashing {len(data)} bytes")

        # Loop through each byte in the 'data' sequence
        # 'i' will be the index (0, 1, 2, ...), which is our EEPROM address
        # 'byte' will be the value of the byte at that index
        for i, byte in enumerate(data):
            # Call the write_byte function from Chapter 3
            # to write 'byte' to address 'i'
            write_byte(i, byte)

            # Print progress occasionally (details skipped)
            if i % 512 == 0:
                print(f"[{i:04X}] → 0x{byte:02X}")

        print("[✓] Done flashing")
```

This loop is the core of the flashing process. `enumerate(data)` provides both the index (`i`) and the value (`byte`) for each item in the sequence. The index `i` naturally serves as the target EEPROM address, starting from 0. The `write_byte(i, byte)` call then uses the hardware connections, address/data lines, and control signals (as discussed in previous chapters) to perform the actual write operation on the chip.

Here's a simplified sequence diagram showing `flash.py` reading `rom.bin` and writing bytes:

![writing](/assets/writing.png)

This diagram shows that `flash.py` simply reads the entire `rom.bin` file into memory and then delegates the actual hardware writing for each byte to the `write_byte` function, using the byte's position in the file as its address.

### Where Does the `rom.bin` File Come From?

The `rom.bin` file isn't created manually byte by byte. It's generated by another script in the project: `make_rom.py`. This script takes the machine code (often assembled from 6502 assembly source code) and other necessary data, places it at the correct addresses within a 32KB block (to match the AT28C256's size), and then saves this 32KB block as the `rom.bin` file.

The `Makefile` in the project defines how this works:

```makefile
# Name of the ROM binary file
ROM_BIN = rom.bin

# Python script to generate the ROM
MAKE_ROM = make_rom.py

# Rule to create the ROM binary file
$(ROM_BIN): $(MAKE_ROM)
	@echo "[+] Generating ROM binary..."
	# Run the make_rom.py script using python3
	python3 $(MAKE_ROM)

# ... rest of the Makefile ...
```

This rule tells the `make` command that to create the file `rom.bin`, it needs to run the `make_rom.py` script. We'll dive into the details of what `make_rom.py` does in the next chapter. For now, just understand that this is the step that *creates* the digital blueprint (`rom.bin`) before `flash.py` *applies* it to the EEPROM.

### Conclusion

The ROM image file (`rom.bin`) is the crucial intermediate step that connects the software you want to run on your 6502 computer to the physical EEPROM chip. It's a raw binary file containing all the bytes (code and data) intended for the EEPROM, arranged sequentially by address. The `flash.py` script reads this file and uses the hardware interface and control signals (from previous chapters) to write each byte to its corresponding address on the EEPROM. This file is generated by a separate script (`make_rom.py`), which we will explore in detail next.

Understanding the ROM image file is essential because it is the source of the information that actually gets programmed onto the chip.

[Next Chapter: ROM Image Generation Script](05_rom_image_generation_script_.md)

---
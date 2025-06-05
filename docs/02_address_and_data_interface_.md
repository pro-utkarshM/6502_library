# Chapter 2: Address and Data Interface

Welcome back! In the [previous chapter](01_eeprom_hardware_connections_.md), we learned about connecting the physical pins of the AT28C256 EEPROM chip to the GPIO pins of your Raspberry Pi. We saw the "wiring diagram" that maps each EEPROM pin function (like Address 0 or Data 7) to a specific Pi GPIO number.

Now that the wires are connected, how does the Raspberry Pi actually *talk* to the EEPROM using these wires? That's where the **Address and Data Interface** comes in.

### Talking to the EEPROM: Addresses and Data

Imagine the EEPROM chip is like a huge library filled with tiny mailboxes. Each mailbox can hold exactly one byte (8 bits) of information. To get a specific piece of information (a byte) from the library, you need two things:

1.  The **address** of the mailbox you want to look inside.
2.  A way to transfer the actual **data** (the contents of the mailbox) in or out.

The Address and Data Interface provides these two things using groups of electrical lines (the wires we connected!).

### Address Lines: Finding the Right Mailbox

The AT28C256 EEPROM has 32,768 individual memory locations (mailboxes), each capable of storing 1 byte. To select just one of these locations, the chip uses **address lines**.

As we saw in Chapter 1, our EEPROM has 15 address lines, labeled A0 through A14. Think of these as the individual numbers on a street address.

*   When the Raspberry Pi wants to talk to location number 0 (the very first mailbox), it sets the signals on A0-A14 to represent the number 0 in binary (all the lines would be LOW).
*   When it wants to talk to location number 1, it sets the signals to represent 1 in binary (A0 LOW, A1 LOW, ..., A14 HIGH).
*   When it wants to talk to location number 100, it sets the signals to represent 100 in binary.
*   And so on, up to the last location, which is 32767 (requiring A0 through A14 to represent this number in binary).

The Raspberry Pi controls the state of these 15 GPIO pins (connected to A0-A14) to "dial up" the desired memory address.

### Data Lines: Sending or Receiving the Mail

Once you've selected a specific mailbox using the address lines, you need to put data in or take data out. This is done using the **data lines**.

Our EEPROM has 8 data lines, labeled I/O0 through I/O7. These 8 lines together form a path for a single byte (8 bits) of data to travel.

*   When the Raspberry Pi wants to **write** a byte to the selected address, it puts the 8 bits of the byte onto the 8 data lines. For example, to write the byte `01001101` (binary for 77 decimal), it would set I/O0 LOW, I/O1 HIGH, I/O2 LOW, I/O3 LOW, I/O4 HIGH, I/O5 HIGH, I/O6 LOW, and I/O7 HIGH.
*   When the Raspberry Pi wants to **read** a byte from the selected address, the EEPROM puts the 8 bits of the byte stored at that location onto the 8 data lines. The Raspberry Pi then reads the state of these 8 lines to get the byte.

These data lines are **bidirectional**, meaning they are used for both sending data *to* the EEPROM (output from the Pi) and receiving data *from* the EEPROM (input to the Pi). The EEPROM uses control signals (which we'll cover in the next chapter) to decide if it should listen for data on these lines (write operation) or put data *onto* these lines (read operation).

### Putting it Together: Accessing a Specific Byte

Let's walk through a simple example: The Raspberry Pi wants to **write** the byte `0x42` (which is 66 in decimal, or `01000010` in binary) to memory address `0x0005` (which is 5 in decimal, or `0000000000101` in binary using 15 bits).

Here's a simplified view of what happens using the Address and Data Interface:

1.  **Set the Address:** The Raspberry Pi sets the 15 address lines (A0-A14) to `0000000000101` binary.
    *   A0 (GPIO2) = HIGH (1)
    *   A1 (GPIO3) = LOW (0)
    *   A2 (GPIO4) = HIGH (1)
    *   A3 through A14 (GPIO5 through GPIO16) = LOW (0)
2.  **Set the Data:** The Raspberry Pi sets the 8 data lines (I/O0-I/O7) to `01000010` binary.
    *   I/O0 (GPIO17) = LOW (0)
    *   I/O1 (GPIO18) = HIGH (1)
    *   I/O2 (GPIO19) = LOW (0)
    *   I/O3 (GPIO20) = LOW (0)
    *   I/O4 (GPIO21) = LOW (0)
    *   I/O5 (GPIO22) = HIGH (1)
    *   I/O6 (GPIO23) = LOW (0)
    *   I/O7 (GPIO24) = LOW (0)
3.  **Signal the Write:** The Raspberry Pi uses the **control signals** (/CE, /OE, /WE) to tell the EEPROM, "Okay, the address is set, the data is set, now write this data byte to this address!" (More on this in the next chapter).

To **read** a byte from address `0x0005`:

1.  **Set the Address:** The Raspberry Pi sets the 15 address lines (A0-A14) to `0000000000101` binary, exactly like in step 1 above.
2.  **Signal the Read:** The Raspberry Pi uses the **control signals** (/CE, /OE, /WE) to tell the EEPROM, "Okay, the address is set, now put the data from this address onto the data lines so I can read it!"
3.  **Read the Data:** The Raspberry Pi configures its data line GPIOs as inputs and reads the state of the 8 data lines (I/O0-I/O7) to get the byte stored at address 5.

### How the Code Uses Address and Data Lines

Let's look at snippets from the `flash.py` script to see how it controls these lines using the GPIO pin numbers defined based on our wiring from Chapter 1.

Remember these definitions from `flash.py`:

```python
import RPi.GPIO as GPIO

ADDR_PINS = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]  # A0-A14
DATA_PINS = [17, 18, 19, 20, 21, 22, 23, 24]                      # D0-D7
# ... control pins defined later ...
```

The `set_address` function in `flash.py` takes an address number (like 5 or 100) and sets the corresponding GPIO pins (defined in `ADDR_PINS`) to the correct HIGH or LOW state to represent that number in binary.

```python
def set_address(addr):
    # This loop goes through each bit position (0 to 14) of the address number
    # and sets the corresponding Pi GPIO pin (from ADDR_PINS) to match the bit.
    for i, pin in enumerate(ADDR_PINS):
        # Check if the i-th bit of 'addr' is 1 or 0 using bitwise operations
        bit_value = (addr >> i) & 1
        # Set the GPIO pin to HIGH (1) or LOW (0)
        GPIO.output(pin, bit_value)

# Example: To set address 5 (binary 101):
# set_address(5)
# This would set GPIO2 (A0) HIGH, GPIO3 (A1) LOW, GPIO4 (A2) HIGH, and the rest LOW.
```

This function directly translates the address number we want to access into the physical electrical signals on the address pins.

Similarly, the `set_data` function takes a byte value and sets the GPIO pins defined in `DATA_PINS` to represent that byte in binary.

```python
def set_data(data):
    # This loop goes through each bit position (0 to 7) of the data byte
    # and sets the corresponding Pi GPIO pin (from DATA_PINS) to match the bit.
    for i, pin in enumerate(DATA_PINS):
        # Check if the i-th bit of 'data' is 1 or 0
        bit_value = (data >> i) & 1
        # Set the GPIO pin to HIGH (1) or LOW (0)
        GPIO.output(pin, bit_value)

# Example: To set data 0x42 (binary 01000010):
# set_data(0x42)
# This would set GPIO17 (I/O0) LOW, GPIO18 (I/O1) HIGH, etc.
```

When reading data, a similar process happens, but the data pins would first be configured as *inputs* on the Raspberry Pi, and then the script would use `GPIO.input(pin)` for each pin in `DATA_PINS` to read the byte presented by the EEPROM. (The provided `flash.py` script focuses on *writing*, but the reading concept is the reverse).

### How the Address and Data Signals Flow (Simplified)

Here's a very simple visual of the Pi setting an address:
![address_flow](/assets/address_flow.png)


And here's the Pi setting data for a write operation:

![data_flow](/assets/data_flow.png)

### Conclusion

The Address and Data Interface is the fundamental way the Raspberry Pi communicates *what* memory location it wants to access and *what* data it wants to put there or get back. The address lines select the specific byte location (like a street address), and the data lines transfer the actual byte of information (like the mail).

However, just setting the address and data isn't enough. The EEPROM needs to know *when* to look at the address, *when* to put data out, and *when* to actually save data. This timing and control are handled by the **control signals**, which we will explore in the next chapter.

[Next Chapter: EEPROM Control Signals](03_eeprom_control_signals_.md)

---
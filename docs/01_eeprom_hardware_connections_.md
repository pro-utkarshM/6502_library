# Chapter 1: EEPROM Hardware Connections

Welcome to the first chapter of the `6502_library` tutorial! In this chapter, we'll cover the absolute first step in working with an AT28C256 EEPROM chip using a Raspberry Pi: making the physical connections.

### Why Hardware Connections?

Imagine you have two people who need to talk to each other, but they are in different rooms and can only communicate by sending signals through a bunch of wires. For their conversation to make sense, they need to agree on which wire means what. Wire #1 might be for talking, Wire #2 for listening, Wire #3 for saying "okay, I received your message", and so on.

Similarly, to communicate between your Raspberry Pi and an AT28C256 EEPROM chip, you need to connect them with wires. The EEPROM is like a storage box for data that keeps its contents even when the power is turned off. Your Raspberry Pi is like the controller that needs to read data from or write data into this box.

The physical connections define which specific "wire" (a connection between a Raspberry Pi GPIO pin and an EEPROM pin) is used for sending an address, sending data, receiving data, or controlling the timing of these actions. Without this agreed-upon wiring, the Raspberry Pi wouldn't know how to tell the EEPROM what to do.

This chapter provides the "wiring diagram" – a map showing exactly which pin on the AT28C256 should be connected to which numbered GPIO pin on your Raspberry Pi. This map is absolutely crucial because the software we'll use later relies on knowing these specific connections to control the EEPROM.

### The Pin Map: Connecting the Raspberry Pi to the AT28C256

The AT28C256 is a memory chip, and it has many pins on its body (it's a Dual In-line Package, or DIP, chip, usually with 28 pins). Each pin has a specific job. The Raspberry Pi has many GPIO (General Purpose Input/Output) pins that can be controlled by software.

We need to connect specific EEPROM pins to specific Pi GPIO pins according to their function:

1.  **Address Lines:** These are like the "address" on a mailbox. The Raspberry Pi uses these pins to tell the EEPROM *which* specific location (byte) inside the memory chip it wants to read from or write to.
2.  **Data Lines:** These are the "mail slot" where the actual data (the byte) goes in or comes out.
3.  **Control Signals:** These are like control buttons or signals (e.g., "start reading now," "start writing now," "chip is ready"). They manage the timing and type of operation (read or write).
4.  **Power and Ground:** Like any electronic device, the EEPROM needs power (VCC) and a ground reference (GND) to operate.

Here is the specific mapping used by the software in this project. You **must** wire your EEPROM to your Raspberry Pi exactly like this for the provided programming script to work correctly.

#### **EEPROM to Raspberry Pi 5 GPIO Mapping**

Remember that the AT28C256 has pins numbered, usually starting from 1 (often marked with a dot or notch on the chip) and going counter-clockwise. Raspberry Pi GPIO pins are identified by their BCM (Broadcom) numbers, not the physical pin number on the Pi's header.

##### **Address Lines (A0–A14):**

These connections allow the Raspberry Pi to select one of the many memory locations (addresses) on the EEPROM. The AT28C256 has 15 address lines (A0 to A14), allowing it to address 2^15 = 32,768 unique memory locations (bytes).

| AT28C256 Pin | Address Line | Pi GPIO |
| :----------- | :----------- | :------ |
| 10           | A0           | GPIO2   |
| 9            | A1           | GPIO3   |
| 8            | A2           | GPIO4   |
| 7            | A3           | GPIO5   |
| 6            | A4           | GPIO6   |
| 5            | A5           | GPIO7   |
| 4            | A6           | GPIO8   |
| 3            | A7           | GPIO9   |
| 2            | A8           | GPIO10  |
| 1            | A9           | GPIO11  |
| 23           | A10          | GPIO12  |
| 22           | A11          | GPIO13  |
| 27           | A12          | GPIO14  |
| 26           | A13          | GPIO15  |
| 25           | A14          | GPIO16  |

##### **Data Lines (I/O0–I/O7):**

These connections are used to send data *to* the EEPROM (when writing) and receive data *from* the EEPROM (when reading). The AT28C256 handles data 8 bits (1 byte) at a time.

| AT28C256 Pin | Data Line | Pi GPIO |
| :----------- | :-------- | :------ |
| 11           | I/O0      | GPIO17  |
| 12           | I/O1      | GPIO18  |
| 13           | I/O2      | GPIO19  |
| 14           | I/O3      | GPIO20  |
| 15           | I/O4      | GPIO21  |
| 16           | I/O5      | GPIO22  |
| 17           | I/O6      | GPIO23  |
| 18           | I/O7      | GPIO24  |

##### **Control Pins:**

These pins control the operation of the EEPROM, telling it *when* to listen for an address, *when* to output data, and *when* to write data. Note that these signals are "active low," indicated by the `/` prefix or a bar above the name (e.g., /CE, /OE, /WE). This means they perform their action when the signal is pulled LOW (close to 0V), and are inactive when HIGH (close to 5V).

| AT28C256 Pin | Signal | Pi GPIO |
| :----------- | :----- | :------ |
| 20           | /CE    | GPIO25  |
| 22           | /OE    | GPIO26  |
| 27           | /WE    | GPIO27  |

*   `/CE` (Chip Enable): Activates the chip. The chip only responds when /CE is low.
*   `/OE` (Output Enable): Allows the chip to output data onto the data lines (typically when reading). When high, the data pins are in a high-impedance state (like being disconnected).
*   `/WE` (Write Enable): Tells the chip to write the data currently on the data pins to the address currently on the address pins. This must be pulsed low for a specific amount of time.

##### **Power and Ground:**

Essential connections to power the chip. The AT28C256 typically operates at 5 volts.

| AT28C256 Pin | Signal | Connect to |
| :----------- | :----- | :--------- |
| 28           | VCC    | 5V on Pi   |
| 14           | GND    | GND on Pi  |

### Why This Specific Mapping in Code?

The software that programs the EEPROM needs to know which physical wire (connected to a Pi GPIO pin) corresponds to Address Line 0, which one is Data Line 7, which is the /WE signal, and so on.

Look at this snippet from the `flash.py` script:

```python
import RPi.GPIO as GPIO

ADDR_PINS = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]  # A0-A14
DATA_PINS = [17, 18, 19, 20, 21, 22, 23, 24]                      # D0-D7
CE = 25
OE = 26
WE = 27
```

This part of the code defines Python lists and variables that hold the *numbers* of the Raspberry Pi GPIO pins that are connected to the EEPROM's Address, Data, and Control lines. For example, `ADDR_PINS[0]` (which is `2`) corresponds to A0, `DATA_PINS[7]` (which is `24`) corresponds to I/O7, and `CE` (which is `25`) corresponds to the /CE signal.

When the script wants to set Address Line A0 high, it will send a command to the Raspberry Pi's GPIO library targeting `GPIO.output(2, 1)`. When it wants to read Data Line I/O7, it will read from `GPIO.input(24)` (after setting the pin as an input). When it wants to activate the Chip Enable, it will set `GPIO.output(25, 0)`.

Here's a tiny example showing how the code uses the `ADDR_PINS` list to set an address:

```python
import RPi.GPIO as GPIO

# Assume setup() has been called and ADDR_PINS is defined as above
# ADDR_PINS = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]

def set_address(addr):
    # Loop through each pin defined in ADDR_PINS
    # The index 'i' corresponds to the address bit (A0, A1, etc.)
    for i, pin in enumerate(ADDR_PINS):
        # Check if the i-th bit of the address 'addr' is 1 or 0
        # (addr >> i) shifts the bits, & 1 isolates the specific bit
        bit_value = (addr >> i) & 1
        # Set the corresponding Raspberry Pi GPIO pin to HIGH (1) or LOW (0)
        GPIO.output(pin, bit_value)

# Example: Set address 5 (binary 101)
# A0 should be 1 (GPIO2), A1 should be 0 (GPIO3), A2 should be 1 (GPIO4), etc.
# set_address(5)
```

This snippet shows how the Python code directly maps the desired address bit (like A0) to the corresponding GPIO pin number (like 2) using the `ADDR_PINS` list defined based on our wiring table. The same principle applies to controlling the Data and Control lines using the `DATA_PINS`, `CE`, `OE`, and `WE` variables.

### Important Considerations

*   **Voltage Levels (Level Shifting):** The AT28C256 is a 5-volt chip, while the Raspberry Pi's GPIO pins typically operate at 3.3 volts. Directly connecting 5V signals to 3.3V inputs on the Pi can damage the Pi. Similarly, the Pi's 3.3V output might not be reliably read as a HIGH by the 5V EEPROM. **Level shifting circuitry is required** between the Pi's GPIOs and the EEPROM if you are using a 5V version of the EEPROM and connecting directly to the Pi's 3.3V GPIOs. You can often find level shifter modules (like bidirectional logic level converters) that handle this. *Ensure you understand the voltage requirements of your specific EEPROM and Pi model!* (Note: Some older AT28C256 chips might be 5V only; newer versions or similar chips might be 3.3V compatible, simplifying this).
*   **Pin States:** As mentioned in the `connections.md` notes, the Pi needs to be able to set pins as outputs (to send address and data, and control signals) and sometimes inputs (to read data *from* the EEPROM). This will be handled by the programming script.

### Conclusion

In this chapter, we learned that connecting the Raspberry Pi to the AT28C256 EEPROM requires a specific "wiring diagram" or pin mapping. We looked at the tables defining which EEPROM pin connects to which Raspberry Pi GPIO pin for address, data, and control signals, as well as power. This physical connection map is the foundation, enabling the software to manipulate the correct pins to communicate with the EEPROM. We also saw how this mapping is reflected in the Python script that will program the EEPROM.

Understanding these physical connections is the first vital step before we dive into how the Raspberry Pi uses these connections to actually talk to the EEPROM.

In the next chapter, we'll take a closer look at the Address and Data lines and how they are used together to specify memory locations and transfer data.

[Next Chapter: Address and Data Interface](02_address_and_data_interface_.md)

---
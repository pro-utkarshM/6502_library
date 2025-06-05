# Chapter 3: EEPROM Control Signals

Welcome back to the tutorial! In the [previous chapters](01_eeprom_hardware_connections_.md), we covered the essential physical connections between your Raspberry Pi and the AT28C256 EEPROM chip ([Chapter 1: EEPROM Hardware Connections](01_eeprom_hardware_connections_.md)), and how the address and data lines work together to select a specific memory location and prepare data for reading or writing ([Chapter 2: Address and Data Interface](02_address_and_data_interface_.md)).

So, you've set the address lines to point to mailbox number 5, and you've put the byte `0x42` onto the data lines. Great! But how does the EEPROM chip know whether you want to *read* what's already in mailbox 5, or *write* `0x42` into mailbox 5? And when exactly should it pay attention to the address and data lines?

This is where the **Control Signals** come in.

### The Conductors of the EEPROM Orchestra

Think of the EEPROM as a highly organized storage facility. The address lines tell you *which* storage locker you're interested in, and the data lines are where you exchange items with that locker. But you need someone to manage the process – to tell the facility when you're ready to access a locker, whether you're putting something *in* (writing) or taking something *out* (reading), and when the operation is finished.

The control signals are these crucial management instructions. They are specific pins on the EEPROM chip that the Raspberry Pi toggles (sets HIGH or LOW) in a precise sequence to tell the chip exactly what operation to perform and when.

The AT28C256 has three main control pins we use:

*   `/CE` (Chip Enable)
*   `/OE` (Output Enable)
*   `/WE` (Write Enable)

Notice the `/` prefix or a line above the name in documentation (like $\overline{\text{CE}}$, $\overline{\text{OE}}$, $\overline{\text{WE}}$). This bar or slash means these signals are **active low**. This is a common concept in electronics: the action happens when the signal is LOW (close to 0 volts), rather than HIGH (close to 5 volts). When the signal is HIGH, it's inactive.

Let's look at each one.

#### `/CE` (Chip Enable): The Master Switch

The `/CE` pin acts like a master power switch or a "Hey, listen up!" signal for the entire chip.

*   When `/CE` is **HIGH**, the EEPROM is basically sleeping or ignoring what's happening on most of its other pins (address, data, other control signals). It consumes very little power in this state.
*   When `/CE` is **LOW**, the EEPROM wakes up and becomes active. It starts paying attention to the address lines and other control signals.

Why have a chip enable? In bigger systems, you might have several memory chips connected to the same address and data lines. The `/CE` signal is how the main controller (like your Raspberry Pi) selects *which* specific chip it wants to talk to at any given moment. Only the chip with its `/CE` pulled LOW will respond. For our setup with just one EEPROM, we typically just pull `/CE` LOW when we want to do anything with the chip (read or write).

In our wiring (from [Chapter 1](01_eeprom_hardware_connections_.md)), `/CE` is connected to Raspberry Pi GPIO 25.

#### `/OE` (Output Enable): Opening the Data Door for Reading

The `/OE` pin controls whether the EEPROM puts data onto the data lines (I/O0-I/O7).

*   When `/OE` is **HIGH**, the EEPROM's data pins are in a "high-impedance" state. Imagine them as being electronically disconnected from the data wires. This is important when the Raspberry Pi needs to control the data lines (like when setting data for a write operation), or when other devices might be sharing the data bus.
*   When `/OE` is **LOW**, and `/CE` is also LOW, the EEPROM will take the byte stored at the address currently on the address lines and actively place its bits onto the data lines (I/O0-I/O7). The Raspberry Pi can then read the voltage levels on these pins to get the byte.

This signal is primarily used during **read operations**.

In our wiring, `/OE` is connected to Raspberry Pi GPIO 26.

#### `/WE` (Write Enable): Telling the Chip to Save Data

The `/WE` pin is the signal that tells the EEPROM to perform a **write operation** – that is, to permanently store the data currently on the data lines into the memory location currently specified by the address lines.

*   When `/WE` is **HIGH**, no write operation is happening.
*   When `/WE` is **LOW**, *while* `/CE` is LOW and `/OE` is HIGH (typically), the EEPROM prepares to write. For the write to actually happen, the `/WE` signal must be pulled **LOW for a specific, short duration** (a "pulse") and then brought back HIGH.

This `LOW` pulse on `/WE` is the trigger for the EEPROM to take the data and address values and start its internal process of saving the data. This internal saving process takes some time (called the write cycle time, which can be milliseconds for an EEPROM byte write, much longer than setting the pins!). The Raspberry Pi must wait for this internal cycle to complete before trying to write again or read from the chip reliably.

In our wiring, `/WE` is connected to Raspberry Pi GPIO 27.

### The Write Cycle: Toggling the Switches in Sequence

Writing a byte to the EEPROM isn't instantaneous like flipping a single switch. It requires a specific sequence of events, timed according to the EEPROM's datasheet (the official technical document for the chip). The `flash.py` script follows this "protocol" precisely.

Let's outline the basic steps the Raspberry Pi takes to write one byte at a specific address using these control signals, address lines, and data lines:

1.  **(Prep) Set Address:** The Raspberry Pi sets the address lines (A0-A14) to the desired memory location ([Chapter 2](02_address_and_data_interface_.md)).
2.  **(Prep) Set Data:** The Raspberry Pi sets the data lines (I/O0-I/O7) to the byte value it wants to write ([Chapter 2](02_address_and_data_interface_.md)). The data pins are acting as *outputs* from the Pi.
3.  **Activate Chip:** Pull `/CE` LOW. The EEPROM wakes up and sees the address and data.
4.  **Disable Output:** Ensure `/OE` is HIGH. This makes sure the EEPROM isn't trying to put data *onto* the data lines at the same time the Pi is putting data *on* them.
5.  **Trigger Write:** Pull `/WE` LOW. This is the signal to begin the write.
6.  **Hold Briefly:** Keep `/WE` LOW for the minimum required pulse width (this is very short, microseconds).
7.  **End Write Trigger:** Pull `/WE` HIGH again. The EEPROM has now latched (captured) the address and data and has started its internal saving process.
8.  **Deactivate Chip:** Pull `/CE` HIGH. This is good practice to put the chip back into a low-power state and deselected, though some datasheets might allow keeping it low during the write cycle.
9.  **Wait:** Wait for the required byte write cycle time to complete (this is the longest step, milliseconds). The EEPROM is busy saving the data internally. Trying to write or read from the chip before this time is up will result in errors or corrupted data.

Here's a simplified sequence diagram showing the control signals during a write operation:
![write_cycle](/assets/write_cycle.png)


This sequence is critical! If the timing or order is wrong, the data won't be written correctly, or the chip might not respond as expected.

### Control Signals in the Code

Let's see how the `flash.py` script implements this write cycle using the GPIO pins connected to the control signals.

Recall the control pin definitions:

```python
import RPi.GPIO as GPIO
# ... other pin definitions ...
CE = 25 # Connected to /CE
OE = 26 # Connected to /OE
WE = 27 # Connected to /WE

WRITE_DELAY = 0.001 # 1 millisecond write cycle delay
# ... rest of the code ...
```

These lines simply assign the Raspberry Pi GPIO numbers for `/CE`, `/OE`, and `/WE` to easy-to-use variable names. `WRITE_DELAY` holds the required time to wait after a write pulse.

The `setup()` function ensures these pins are configured as outputs and initially set HIGH (inactive state).

```python
def setup():
    GPIO.setmode(GPIO.BCM)
    # ... setup address and data pins ...
    # Set control pins as outputs and initially HIGH (inactive)
    GPIO.setup(CE, GPIO.OUT)
    GPIO.setup(OE, GPIO.OUT)
    GPIO.setup(WE, GPIO.OUT)

    # Ensure control signals are inactive initially
    GPIO.output(CE, 1) # /CE HIGH (chip disabled)
    GPIO.output(OE, 1) # /OE HIGH (outputs disabled)
    GPIO.output(WE, 1) # /WE HIGH (write disabled)
```

This sets the initial state, making sure the EEPROM isn't accidentally activated or triggered when the script starts.

Now, let's look at the core `write_byte` function, which performs the sequence described above for a single byte:

```python
def write_byte(addr, data):
    # 1. Set Address (using function from Chapter 2)
    set_address(addr)

    # 2. Set Data (using function from Chapter 2)
    set_data(data)

    # 3. Activate Chip: Pull /CE LOW
    GPIO.output(CE, 0) # Set GPIO 25 LOW

    # 4. Disable Output: Keep /OE HIGH (Important for writing)
    GPIO.output(OE, 1) # Set GPIO 26 HIGH

    # 5. Trigger Write: Pull /WE LOW
    GPIO.output(WE, 0) # Set GPIO 27 LOW

    # 6. Hold Briefly: Wait a very short time for the pulse (microseconds)
    # The RPi's GPIO speed is often sufficient, but a tiny sleep ensures
    # the LOW pulse is registered by the chip.
    time.sleep(0.00001) # 10 microseconds

    # 7. End Write Trigger: Pull /WE HIGH
    GPIO.output(WE, 1) # Set GPIO 27 HIGH

    # 8. Deactivate Chip: Pull /CE HIGH
    GPIO.output(CE, 1) # Set GPIO 25 HIGH

    # 9. Wait: Wait for the internal write cycle to complete (milliseconds)
    time.sleep(WRITE_DELAY) # Wait 1 millisecond (or value from datasheet)

# ... rest of the flash_rom function which calls write_byte ...
```

This snippet shows how the GPIO functions (`GPIO.output`) are used with the control pin variables (`CE`, `OE`, `WE`) to cycle through the required steps of the write protocol. The `time.sleep` calls are crucial for meeting the timing requirements specified in the EEPROM datasheet, particularly the pulse width for `/WE` and the longer wait time for the internal write cycle (`WRITE_DELAY`). The value `0.001` (1 millisecond) for `WRITE_DELAY` is a common value found in datasheets for this type of EEPROM, though checking the specific chip's datasheet is always recommended.

### Conclusion

Control signals (/CE, /OE, /WE) are like the director's baton, orchestrating the communication between the Raspberry Pi and the EEPROM. They tell the chip when to become active, whether to prepare for reading or writing, and precisely when to latch and save data. Understanding their purpose and the required sequence (especially the write cycle) is vital for successfully programming the EEPROM. The `flash.py` script implements this sequence by carefully toggling the Raspberry Pi GPIO pins connected to these control signals, ensuring the timing meets the EEPROM's requirements.

With the hardware connected, the Address and Data interfaces understood, and the role of Control Signals clear, we now know *how* the Raspberry Pi talks to the EEPROM at a low level. The next step is to understand *what* data we actually want to put onto the EEPROM – the ROM image file.

[Next Chapter: ROM Image File](04_rom_image_file_.md)

---
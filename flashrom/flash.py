import RPi.GPIO as GPIO
import time

ADDR_PINS = [2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]  # A0-A14
DATA_PINS = [17, 18, 19, 20, 21, 22, 23, 24]                      # D0-D7
CE = 25
OE = 26
WE = 27

WRITE_DELAY = 0.001  # Delay for EEPROM write (in seconds)

def setup():
    GPIO.setmode(GPIO.BCM)
    for pin in ADDR_PINS + DATA_PINS + [CE, OE, WE]:
        GPIO.setup(pin, GPIO.OUT)
    GPIO.output(CE, 1)
    GPIO.output(OE, 1)
    GPIO.output(WE, 1)

def set_address(addr):
    for i, pin in enumerate(ADDR_PINS):
        GPIO.output(pin, (addr >> i) & 1)

def set_data(data):
    for i, pin in enumerate(DATA_PINS):
        GPIO.output(pin, (data >> i) & 1)

def write_byte(addr, data):
    set_address(addr)
    set_data(data)
    GPIO.output(CE, 0)
    GPIO.output(OE, 1)
    GPIO.output(WE, 0)
    time.sleep(0.00001)
    GPIO.output(WE, 1)
    GPIO.output(CE, 1)
    time.sleep(WRITE_DELAY)

def flash_rom(filename):
    with open(filename, "rb") as f:
        data = f.read()
        print(f"[+] Flashing {len(data)} bytes")
        for i, byte in enumerate(data):
            write_byte(i, byte)
            if i % 512 == 0:
                print(f"[{i:04X}] → 0x{byte:02X}")
        print("[✓] Done flashing")

def cleanup():
    GPIO.cleanup()

if __name__ == "__main__":
    try:
        setup()
        flash_rom("rom.bin")
    finally:
        cleanup()

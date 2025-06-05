# Tutorial: 6502_library

This project is about **programming** an EEPROM chip, specifically an AT28C256,
using a *Raspberry Pi*. It involves *generating* a binary file containing 6502
program code and then using a Python script to *write* that code onto the
EEPROM byte by byte, controlling the chip's pins precisely according to the
hardware setup and required timings.


## Visual Overview
![overview](/assets/overview.png)

## Chapters

1. [EEPROM Hardware Connections
](docs/01_eeprom_hardware_connections_.md)
2. [Address and Data Interface
](docs/02_address_and_data_interface_.md)
3. [EEPROM Control Signals
](docs/03_eeprom_control_signals_.md)
4. [ROM Image File
](docs/04_rom_image_file_.md)
5. [ROM Image Generation Script
](docs/05_rom_image_generation_script_.md)
6. [EEPROM Programmer Script
](docs/06_eeprom_programmer_script_.md)

---

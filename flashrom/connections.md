## **EEPROM to Raspberry Pi 5 GPIO Mapping**

### **Address Lines (A0–A14):**

| AT28C256 Pin | Address Line | Pi GPIO |
|--------------|--------------|---------|
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

---

### **Data Lines (I/O0–I/O7):**

| AT28C256 Pin | Data Line | Pi GPIO |
|--------------|-----------|---------|
| 11           | I/O0      | GPIO17  |
| 12           | I/O1      | GPIO18  |
| 13           | I/O2      | GPIO19  |
| 14           | I/O3      | GPIO20  |
| 15           | I/O4      | GPIO21  |
| 16           | I/O5      | GPIO22  |
| 17           | I/O6      | GPIO23  |
| 18           | I/O7      | GPIO24  |

---

### **Control Pins:**

| AT28C256 Pin | Signal | Pi GPIO |
|--------------|--------|---------|
| 20           | /CE    | GPIO25  |
| 22           | /OE    | GPIO26  |
| 27           | /WE    | GPIO27  |

---

### **Power and Ground:**

| AT28C256 Pin | Signal | Connect to   |
|--------------|--------|--------------|
| 28           | VCC    | 5V on Pi     |
| 14           | GND    | GND on Pi    |

---

### **Important Notes:**

- **Level shifting is required** if you’re using the Pi’s 3.3V GPIOs with the 5V EEPROM.
- Keep address and data lines as outputs during write.
- Switch data lines to inputs during read.
- Follow AT28C256 datasheet timing requirements (especially for /WE pulse width and write delay).

---

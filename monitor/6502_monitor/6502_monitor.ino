const char ADDR[] = {22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52};
const char DATA[] = {39, 41, 43, 45, 47, 49, 51, 53};

const char PB[] = {23, 25, 27, 29, 31, 33, 35, 37};
const char PA[] = {4, 5, 6, 7, 8, 9, 10, 11};

#define CLOCK 2
#define READ_WRITE 3

void setup() {
  for (int n = 0; n < 16; n += 1) {
    pinMode(ADDR[n], INPUT);
  }
  for (int n = 0; n < 8; n += 1) {
    pinMode(DATA[n], INPUT);
  }

  for (int n = 0; n < 8; n += 1) {
    pinMode(PA[n], INPUT);
  }
    for (int n = 0; n < 8; n += 1) {
    pinMode(PB[n], INPUT);
  }
  
  pinMode(CLOCK, INPUT);
  pinMode(READ_WRITE, INPUT);

  attachInterrupt(digitalPinToInterrupt(CLOCK), onClock, RISING);
  
  Serial.begin(57600);
}

void onClock() {
  char output[15];

  unsigned int pa = 0;
  for (int n = 0; n < 8; n += 1) {
    int bit = digitalRead(PA[n]) ? 1 : 0;
    Serial.print(bit);
    pa = (pa << 1) + bit;
  }

  Serial.print("   ");

  unsigned int pb = 0;
  for (int n = 0; n < 8; n += 1) {
    int bit = digitalRead(PB[n]) ? 1 : 0;
    Serial.print(bit);
    pb = (pb << 1) + bit;
  }

  Serial.print("   ");


  unsigned int address = 0;
  for (int n = 0; n < 16; n += 1) {
    int bit = digitalRead(ADDR[n]) ? 1 : 0;
    Serial.print(bit);
    address = (address << 1) + bit;
  }
  
  Serial.print("   ");
  
  unsigned int data = 0;
  for (int n = 0; n < 8; n += 1) {
    int bit = digitalRead(DATA[n]) ? 1 : 0;
    Serial.print(bit);
    data = (data << 1) + bit;
  }

  sprintf(output, "   %04x  %c %02x", address, digitalRead(READ_WRITE) ? 'r' : 'W', data);
  Serial.println(output);  
}

void loop() {
}

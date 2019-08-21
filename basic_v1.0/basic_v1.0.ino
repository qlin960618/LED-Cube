
#define SER1    3
#define SERCLK1 4
#define SER2    5
#define SERCLK2 6
#define VSER    7
#define VSERCLK 8
#define RCLK    9

uint8_t bArray[64];
byte outArray[4][8][8];


void regLatch()
{
  digitalWrite(RCLK, HIGH);
  digitalWrite(RCLK, LOW);
  
}

void shiftOutV(uint8_t layer)
{
  static uint8_t player=9;
  if (layer>7)
    return;
  if(player+1==layer)
  {
    digitalWrite(VSERCLK, HIGH);
    digitalWrite(VSERCLK, LOW);
  }else
  {
    shiftOut(VSER, VSERCLK, MSBFIRST, (0x01<<layer));
  }
}

void shiftOutC(uint8_t b1, uint8_t b2, uint8_t bitOrder)
{
  uint8_t mask;
  if(bitOrder==LSBFIRST)
    mask = 0x01;
  else
    mask = 0x80;
    
  for(uint8_t i=0; i<8; i++)
  {
    digitalWrite(SER1, (b1 & mask));
    digitalWrite(SER2, (b2 & mask));
    if(bitOrder==LSBFIRST)
      mask = mask<<1;
    else
      mask = mask>>1;
    digitalWrite(SERCLK1, HIGH);
    digitalWrite(SERCLK2, HIGH);
    digitalWrite(SERCLK1, LOW); 
    digitalWrite(SERCLK2, LOW); 
  }
  
}

void setup() {

  
  Serial.begin(9600);
  pinMode(SER1,     OUTPUT);
  pinMode(SERCLK1,  OUTPUT);
  pinMode(SER2,     OUTPUT);
  pinMode(SERCLK2,  OUTPUT);
  pinMode(VSER,     OUTPUT);
  pinMode(VSERCLK,  OUTPUT);
  pinMode(RCLK,     OUTPUT);


  Serial.print("Program Begin");
}


int slay=0;

void loop() {
  shiftOutC(0x09, 0x0c, LSBFIRST);
  shiftOutV(slay);

  slay++;
  if (slay>7)
    slay=0;

  regLatch();
  
  delay(1000);
  
}

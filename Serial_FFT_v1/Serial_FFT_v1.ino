#include<Timer.h>

#define SER1    3
#define SERCLK1 4
#define SER2    5
#define SERCLK2 6
#define VSER    7
#define VSERCLK 8
#define RCLK    9

Timer ut1;

uint8_t bArray[64];
byte outArray[4][8][8];


void regLatch()
{
  /*
  digitalWrite(RCLK, HIGH);
  digitalWrite(RCLK, LOW);
  */
  PORTB|=0x02;
  PORTB&=~0x02;  
}

void shiftOutVLayer(uint8_t val)
{
  uint8_t i;
  for (i = 0; i < 8; i++)  {
    if(val & (1 << (7 - i)))
      PORTD |= 0x80;
    else
      PORTD &= ~0x80;
      
    PORTB|=0x01;
    PORTB&=~0x01;  
  }
}

void shiftOutV(uint8_t layer)
{
  static uint8_t lastLayer=9;
  if (layer>7)
    return;
  if(lastLayer+1==layer)
  {
    /*
    digitalWrite(VSERCLK, HIGH);
    digitalWrite(VSERCLK, LOW);
    */
    PORTB|=0x01;
    PORTB&=~0x01;
  }else
  {
    shiftOutVLayer((0x01<<layer));
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
    /*
    digitalWrite(SER1, (b1 & mask));
    digitalWrite(SER2, (b2 & mask));
    */
    if(b1 & mask)
      PORTD|=0x08;
    else
      PORTD&=~0x08;
    if(b2 & mask)
      PORTD|=0x20;
    else
      PORTD&=~0x20;
    
    if(bitOrder==LSBFIRST)
      mask = mask<<1;
    else
      mask = mask>>1;
     /*
    digitalWrite(SERCLK1, HIGH);
    digitalWrite(SERCLK2, HIGH);
    digitalWrite(SERCLK1, LOW); 
    digitalWrite(SERCLK2, LOW); 
    */
    PORTD|=0x50;
    PORTD&=~0x50;
  }
  
}

int vcount=0;
int gcount=0;

ISR(TIMER1_COMPA_vect)
{
    ///////shift out
    shiftOutV(vcount);
    shiftOutC(outArray[gcount][vcount][3],outArray[gcount][vcount][7], LSBFIRST);
    shiftOutC(outArray[gcount][vcount][2],outArray[gcount][vcount][6], LSBFIRST);
    shiftOutC(outArray[gcount][vcount][1],outArray[gcount][vcount][5], LSBFIRST);
    shiftOutC(outArray[gcount][vcount][0],outArray[gcount][vcount][4], LSBFIRST);

    //////////latch register
    regLatch();
    
    if (vcount<7)
      vcount++;
    else
    {
      vcount=0;
      gcount++;
      if (gcount>3)
        gcount=0;
    }
}

const bool gMat[5][4]={{0,0,0,0},{0,0,0,1},{1,0,1,0},{1,1,1,0},{1,1,1,1}};

void updateArray()
{ 


  /*
  for(int i=0; i<64; i++)
  {
    int kk=bArray[i];
    kk+=random(7)-3;
    if(kk>39) kk=39;
    if(kk<0) kk=0;
    bArray[i]=kk;
  }
  */

  
  for(int v=0; v<8; v++)    //v
  {
    //Serial.print("\t\t"); Serial.println(v);
    for(int j=0; j<8; j++)  //row
    {
      int ind= j*8;
      byte in[4]={0x00,0x00,0x00,0x00};
      for(int k=0; k<8; k++)    //col 
      {
        int gInd = bArray[ind+k]-v*5;
        if(gInd<0) gInd=0;
        if (gInd>4) gInd=4;
                
        in[0] = in[0] | ( ((byte)gMat[gInd][0]) << k );
        in[1] = in[1] | ( ((byte)gMat[gInd][1]) << k );
        in[2] = in[2] | ( ((byte)gMat[gInd][2]) << k );
        in[3] = in[3] | ( ((byte)gMat[gInd][3]) << k );
        
      }
      outArray[0][v][j]=in[0];
      outArray[1][v][j]=in[1];
      outArray[2][v][j]=in[2];
      outArray[3][v][j]=in[3];
     }
  }
}

void setup() {

  
  for(int k=0; k<64; k++)
  {
    bArray[k]=15;
  }
  
  //bArray[0]=20;
  
  Serial.begin(9600);
  pinMode(SER1,     OUTPUT);
  pinMode(SERCLK1,  OUTPUT);
  pinMode(SER2,     OUTPUT);
  pinMode(SERCLK2,  OUTPUT);
  pinMode(VSER,     OUTPUT);
  pinMode(VSERCLK,  OUTPUT);
  pinMode(RCLK,     OUTPUT);

  ut1.every(60,updateArray);

  Serial.println("Program Begin");
  
  //////attach refresh interrupt  
  TCCR1A = 0; // set entire TCCR1A register to 0
  TCCR1B = 0;
  TCNT1  = 0; //initialize counter value to 0
  OCR1A=40;   // 1.56kHz      Min 4: at 15.6kHz
  TCCR1B |= (1 << CS12) | (1 << WGM12);   // set rescaler to 256
  TIMSK1 |= (1 << OCIE1A);  //enable overflow
}

int posCounter=0;

void loop() {
  ut1.update();
  if(Serial.available())
  {
    uint8_t c=Serial.read();
    if(c==0xff)
    {
      posCounter=0;
      //bArray[0]=40;
    }
    else if(c<=100)
    {
      bArray[posCounter++]=c;
      if(posCounter>=64) posCounter=0;
    }
  }
  
}

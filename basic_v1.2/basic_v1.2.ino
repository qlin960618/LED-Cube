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
  for(int i=0; i<64; i++)
  {
    int kk=bArray[i];
    kk+=random(7)-3;
    if(kk>39) kk=39;
    if(kk<0) kk=0;
    bArray[i]=kk;
  }

  
  for(int v=0; v<8; v++)    //v
  {
    //Serial.print("\t\t"); Serial.println(v);
    for(int j=0; j<8; j++)  //row
    {
      byte in[4]={0x00,0x00,0x00,0x00};
      for(int k=0; k<8; k++)    //col 
      {
        int ind= j*8+k ;
        int gInd = bArray[ind]-v*5;
        if(gInd<0) gInd=0;
        if (gInd>4) gInd=4;
        //Serial.print(ind); Serial.print("\t"); Serial.println(gInd);

        
        for(int g=0; g<4; g++)
        {
         // Serial.print("\t\t\t\t"); Serial.println(gMat[gInd][g]?"True":"False");
          in[g] = in[g] | ( ((byte)gMat[gInd][g]) << k );
        }
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
    bArray[k]=k/2;
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
  OCR1A=60;   // 700Hz
  TCCR1B |= (1 << CS12) | (1 << WGM12);   // set rescaler to 256
  TIMSK1 |= (1 << OCIE1A);  //enable overflow
}


void loop() {
  ut1.update();
  delay(1);
  
}

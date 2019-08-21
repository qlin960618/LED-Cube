#include<Timer.h>
#define SHIFT_LATCH 5
#define SERIAL 7
#define SERIAL_CK 6
#define VSERIAL 9
#define VSERIAL_CK 8
//#define SAMPLES 128
#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft
#include <FFT.h> // include the library


const byte adcPin = 0;  // A0 Audio in pin

/////main Boolean array
//int bArray[25];


bool mat[4][4]={{0,0,0,1},{1,0,1,0},{1,1,1,0},{1,1,1,1}};// brightness pattern

byte outArray[4][5][4];///LED runthrough byte array

int SAMPLEScnt;

Timer ut1;/////initilize refresh timer

////////for debugging
void printBits(byte myByte){
  for(byte mask = 0x80; mask; mask >>= 1){
    if(mask  & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
}

/////////////LED runthrough
int vcount=0;
int gcount=0;
ISR(TIMER0_COMPA_vect)
{
    //Serial.println(millis());
    byte v = 1;
    ////////increament v byte
    v<<=(vcount);

    ///////shift out
    shiftOut(VSERIAL, VSERIAL_CK, MSBFIRST, v);
    shiftOut(SERIAL, SERIAL_CK, LSBFIRST, outArray[gcount][vcount][3]);
    shiftOut(SERIAL, SERIAL_CK, LSBFIRST, outArray[gcount][vcount][2]);
    shiftOut(SERIAL, SERIAL_CK, LSBFIRST, outArray[gcount][vcount][1]);
    shiftOut(SERIAL, SERIAL_CK, LSBFIRST, outArray[gcount][vcount][0]);

    //////////latch register
    digitalWrite(SHIFT_LATCH, HIGH);
    delayMicroseconds(1);
    digitalWrite(SHIFT_LATCH, LOW);
    
    if (vcount<4)
      vcount++;// next V level
    else
    {
      vcount=0;
      if (gcount>=3)//next brightness cycle
        gcount=0;
      else
        gcount++;
    }
}


//////Data sampling
ISR (ADC_vect)
{
  if (SAMPLEScnt >= FFT_N*2)
  {
    SAMPLEScnt=0;
    ADCSRB = 0;  // Timer/Counter1 Compare Match A Disable (Disable ADC)
  }
  else
  {  
    fft_input[SAMPLEScnt] = ADC;
    fft_input[SAMPLEScnt+1]=0;
    SAMPLEScnt+=2;
    
  }
}  // end of ADC_vect
  
EMPTY_INTERRUPT (TIMER1_COMPB_vect);
 

//const int bArrayHist[25]= {20, 14, 14, 14, 12, 8, 8, 7, 8, 6, 7, 7, 7, 3, 2, 2, 3, 2, 2, 2, 3, 2, 1, 2, 2};
//const int bArrayHist[25]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
const int bArrayHist[25]= {18,17,17,16,15, 12, 12, 11, 11, 9, 10, 10, 10, 6, 5, 6, 5, 6, 6, 4, 5, 5, 6, 3, 4};
//;//={7,5,2,2,7,4, 0, 3 ,4, 2, 0, 3, 4, 2, 0, 0, 3, 2, 0, 2, 0, 0, 0, 0, 0};
//15  11  11  11  10  7 7 6 7 6 6 6 6 3 2 2 3 2 2 2 3 2 1 2 2
void updateArray()
{
  //Serial.println("Processsing");
  /////////////////FFT Processing

    fft_window(); // window the data for better frequency response
    fft_reorder(); // reorder the data before doing the fft
    fft_run(); // process the data in the fft
    fft_mag_log(); // take the output of the fft


      int bArray[25];////clear first in array
      //int bArray1[25];
      //bArray1[0]=0;
      bArray[0]=0;
      int h=0;
      for(int i=10; i<64; i+=2)
      {
        bArray[h]=abs((fft_log_out[i]+fft_log_out[i+1])/3 - bArrayHist[h]);
        
   //     Serial.print(bArray[h]);
     //   Serial.print(" ");
        h++;
        if(h<=24)
          bArray[h]=0;
            
        if (h==25)
          break;
      }  
     // Serial.println();

      /*for(int i=0; i<25; i++)
      {
        Serial.print(bArray[i]);
        Serial.print("\t");
        //bArray[i]=bArray[i]-bArrayHist[i];
      }
      Serial.println();
      */

      
      for (int m=0; m<5; m++)
      {
        int k=0; ////k = k/8 ////count = k%8
        for (int i=0; i < 25; i++)
        {
            
            if(bArray[i]>(m*4))
            {
              int a=bArray[i]-m*4-1;
              if (a>3) a=3;
              for (int g=0; g<4; g++)/////Update all four brightness array
                outArray[g][m][k/8]=(outArray[g][m][k/8]<<1)|mat[a][g];
            }
            else
              for(int g=0; g<4; g++) /////LED off for all brightness array
                outArray[g][m][k/8]=(outArray[g][m][k/8]<<1);
            k++;
        }
        for(int g=0; g<4; g++)
          outArray[g][m][3]<<=7;
      }

  ///////reenable ADC
  ADCSRB = bit (ADTS0) | bit (ADTS2);  // Timer/Counter0 Compare Match A

}

void setup() {
  /////set pin mode
  pinMode(SERIAL_CK, OUTPUT);
  pinMode(SERIAL,OUTPUT);
  pinMode(SHIFT_LATCH,OUTPUT);
  pinMode(VSERIAL,OUTPUT);
  pinMode(VSERIAL_CK,OUTPUT);
  
  ///////initalize for all to be off
  digitalWrite(SHIFT_LATCH, LOW);
  for (int i=0; i<4; i++)
    shiftOut(SERIAL, SERIAL_CK, LSBFIRST, 0x00);
  shiftOut(VSERIAL, VSERIAL_CK, LSBFIRST, 0x00);
  delay(1);
  digitalWrite(SHIFT_LATCH, HIGH);
  delay(1);
  digitalWrite(SHIFT_LATCH, LOW);

  //////attach refresh interrupt  to timer0
  TCCR0A = 0; // set entire TCCR0A register to 0
  TCCR0B = 0;
  TCNT0  = 0; //initialize counter value to 0
  OCR0A=180; // 70Hz 1389Hz total
  TCCR0B |= bit (CS01) | bit (CS00); // set rescaler to 64/
  TIMSK0 |= bit (OCIE0A);  //match A


  // attach DAC to timer1
  // reset Timer 1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  TCCR1B = bit (CS11) | bit (WGM12);  // CTC, prescaler of 8
  TIMSK1 = bit (OCIE1B);  //timer1 CRB match flags
  OCR1A = 40;   //?? need CRA really?
  OCR1B = 40;   // - sampling frequency 50 kHz     16mhz/(8*40)

  ADCSRA =  bit (ADEN) | bit (ADIE) | bit (ADIF);   // turn ADC on, want interrupt on completion
  ADCSRA |= bit (ADPS2);        // Prescaler of 16
  ADMUX = bit (REFS0) | (adcPin & 7);
  ADCSRB = bit (ADTS0) | bit (ADTS2);  // Timer/Counter1 Compare Match B
  ADCSRA |= bit (ADATE);   // turn on automatic triggering


  
  //setup update timer
  ut1.every(30,updateArray);
  
  Serial.begin(9600);
  Serial.println("program begin");
  
}

void loop() {
  
  ut1.update();
  
  
}


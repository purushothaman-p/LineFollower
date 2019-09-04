#include<avr/io.h>
#include<avr/interrupt.h>
/*
 * Binary Values Used
 * Address: 01001101
 * ON: 01001111
 * OFF:01000110
 * INC_BRIGHT:01101010
 * DEC_BRIGHT:01001010
 * 
 * Hexadecimal Values
 * ON: B24DF20D
 * OFF:B24D629D
 * INC_BRIGHT:B24D56A9
 * DEC_BRIGHT:B24D52AD
 */
uint32_t code_ON=0b10110010010011011111001000001101;
uint32_t code_OFF=0b10110010010011010110001010011101;
uint32_t code_INC_BRIGHT=0b10110010010011010101011010101001;
uint32_t code_DEC_BRIGHT=0b10110010010011010101001010101101;
uint32_t code;

volatile int count=0;


ISR(TIMER0_COMPA_vect) {
  PORTD^=(1<<2);                         //Toggle LED 
}

ISR(TIMER1_COMPA_vect) {
  TCCR1B=0x00;                           //Timer 1 OFF
  TCCR0B=0x00;                           // TImer 0 OFF
  PORTD=0x00;                            // LED OFF
}

ISR(TIMER2_OVF_vect) {
  ++count;                               // 
  if(count>=2) {                         //
    count=0;                             //Count resetted
    TCCR2B=0x00;                         //Timer 2 OFF
  }
}

void start_pulse() {
  OCR1A=18000;                           //To cause 9ms delay
  TCCR1B=(1<<CS11);                      // 8 bit prescaling
  TCCR0B=(1<<CS00);                      // No prescaling
  TCNT1=0;                               
  TCNT0=0;
  PORTD=0b00000100;                      //LED turned OFF
  while(TCCR1B&(1<<CS11));               //wait untill 9ms is over
}

void start_delay() { 
  PORTD=0x00;                            //LED OFF
  OCR1A=9000;                            //4.5ms delay
  TCCR1B=(1<<CS11);                     
  TCNT1=0;
  while(TCCR1B&(1<<CS11));               //wait untill 9ms is over
}

void code_pulse() {
  OCR1A=9000;                            //562.5us pulse
  TCCR1B=(1<<CS10);                      //No prescaling
  TCCR0B=(1<<CS00);                      //No prescaling
  TCNT1=0;
  TCNT0=0;
  PORTD=0b00000100;                      //LED Turned ON
  while(TCCR1B&(1<<CS10));
}

void delay_bit(int time_tic) {
  PORTD=0x00;                               
  OCR1A=time_tic;                        //
  TCCR1B=(1<<CS10);                      //No prescaling
  TCNT1=0;
  while(TCCR1B&(1<<CS10));
}

void delay_time() {
  PORTD=0x00;                            //LED OFF
  TCCR2B=(1<<CS22)|(1<<CS21)|(1<<CS20);  //1024 prescaling
  TCNT2=0;
  while(TCCR2B&(1<<CS20));
}

void send_code(int code_value) {
  if(code_value==1) {
    code=code_ON;
  }
  else if(code_value==0) {
     code=code_OFF;
  }
  else if(code_value==2) {
     code=code_INC_BRIGHT;
  }
  else if(code_value==3) {
     code=code_DEC_BRIGHT;
  }
  start_pulse();                          //Start pulse
    start_delay();                        //Start delay
    for(int i=0;i<32;++i) {
      code_pulse();                       //562.5us pulse burst
      if(code&0x80000000) {              
        delay_bit(27000);                 //If its '1' give 1.68ms delay 
      }
      else { 
        delay_bit(9000);                  // If '0' give 562.5us delay
      }
      code<<=1;                           //Left shift code by 1
    }
    code_pulse();                        //EOF
    code=0;
}

int main() {
  TCCR0A=(1<<WGM01);                     //Timer 0 CTC Mode
  TCCR1A=(1<<WGM12);                     //Timer 1 CTC Mode
  TIMSK0=(1<<OCIE0A);                    //Timer0 Interrupt on Compare Match A
  TIMSK1=(1<<OCIE1A);                    //Timer1 Interrupt on Compare Match A
  TIMSK2=(1<<TOIE2);                     //Timer2 overflow
  OCR0A=210;                         //50% duty cyle for 38kHz
  sei();                                 //enable global interrupts
  DDRD=0b00000100;                       //LED Pin output
  PORTD=0x00;                            // LED initially OFF
  Serial.begin(9600);                   
  while(1) {
    send_code(0);
    delay_time();
    send_code(1);
    delay_time();
    send_code(2);
    delay_time();
    send_code(3);
    delay_time();   
  }
  PORTD=0x00;
  return 0;
}


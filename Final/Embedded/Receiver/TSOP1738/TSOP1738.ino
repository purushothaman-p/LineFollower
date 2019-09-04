#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

volatile int count=0;                            //counter variable 
volatile int timer_value=0;                      //to get the time between two falling edge
volatile int pulse_count=-1;                      // to count pulse recieved
int sof=0;                                       //to flag SOF 
volatile uint32_t msg_bit=0;                     //To store the incoming msg
volatile uint32_t new_key=0;                     //To store the complete msg

ISR(TIMER0_COMPA_vect) {                         //To count number of ms
  if(count<50) {
    ++count;
  }
}
  
ISR(INT0_vect) {                                   //Falling edge int
  timer_value=count;                               //Gets the time b/w 2 falling edge interrupt
  count=0;                                         //reset the time
  TCNT0=0;                                         //resets the TIMER counter
  if(sof) {                                        //to mark SOF
    ++pulse_count;                                 
  }

  if(timer_value>=13&&timer_value<=15) {          //if time lies b/w this then SOF is encountered
    msg_bit=0;                                    //Reset the message back to 0 to accept new incoming msg
    sof=1;                                         //Mark sof as 1
    pulse_count=-1;                               //Pulse count back to -1
  }
  else if((pulse_count>=0) && (pulse_count<=31)) { 
    if(timer_value>=2) {                          // received code ia 1
      msg_bit|=(uint32_t)1<<(31-pulse_count);     // mask (31-pulse_cunt) as 1
    }
    else {                                        // else 0 is rewceived   zero is automatically masked.
    }
    if(pulse_count==31) {                         // condition when all the 32 msg bit has been received.
      EICRA=(1<<ISC00);                           // change the interrupt to any logic change.
    }
  }
  else if(pulse_count>=32)                        // condition after stop of frame is received.
    {
        new_key = msg_bit;                        // give new_key = msg_bit (full received msg)
        pulse_count = -1;                         // set back to initial ,  pulse_count= -1
        sof=0;                                    // set back to initial ,  sof=0;
        EICRA=(1<<ISC01);                         // set  interrupt back to falling edge.
    }
}

void setup_external_interrupt() {                 // initialise the interrupt INT0.
  EICRA=(1<<ISC01);                               // enable falling edge interrupt.
  EIMSK=(1<<INT0);                                // enable INT0(PD2) as external interrupt.
}

void init_timer0() {        
  TCCR0A=(1<<WGM01);                              // CTC mode
  TIMSK0=(1<<OCIE0A);                             // enable compare match interrupt channel 1
  OCR0A=250;                                      // set OCR0A as 250 to get interrupt every 1 ms.
}

void timer0_start() {
  TCCR0B=(1<<CS01)|(1<<CS00);                     // start timer 0 with prescaller 64
  TCNT0=0;                                        // clear TCNT0
}

void timer0_stop() {
  TCCR0B=0x00;                                    // stops timer 0
  
}

int main() {
  
  DDRD=0x00;
  
  setup_external_interrupt();

  init_timer0();
  timer0_start();
  sei();
  
  Serial.begin(9600);
  while (1) {
    if(new_key) {                                        // check for received key 
      Serial.println(new_key,HEX);                       // if there is received key print it.
       new_key=0;                                        // make your key zero again.
    }
    //_delay_ms(1000);
    
  }
  return 0;
}



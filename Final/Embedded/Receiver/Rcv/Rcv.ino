/*
   This program recieves the IR Signal which follows the NEC Protocol.
   It uses TSOP 1738 module to detect IR signal, decode the signal received
   and then take the necessary action as per the code.

   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
                REMOTE CODE USED
   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Binary Values Used
        Address: 01001101
        LEFT: 01001111
        RIGHT:01000110
        FOR_LEFT:01101010
        FOR_RIGHT:01001010
        FORWARD: 01100110
        STABLE:11111111

    Hexadecimal Values
        LEFT: B24DF20D
        RIGHT: B24D629D
        FOR_LEFT: B24D56A9
        FOR_RIGHT: B24D52AD
        FORWARD: B24D6699
        STABLE: B24DFF00



    Address of the TSOP: B24D
*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
        HEADER FILES
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include<avr/io.h>
#include<avr/interrupt.h>
#include<util/delay.h>

volatile int count = 0;                                       // To count the number of times signal goes from HIGH to LOW

volatile int timer_value = 0;                                 // To hold the time duration between 2 signal transition( First HIGH to LOW to next HIGH to LOW)

volatile int pulse_count = -1;                                //To count number of pulses recieved out of 32 (since transmitter sends 32 pulse of IR signal which contains data)

int sof = 0;                                                  //To flag whether SOF has been recieved or not. (Start of frame)

volatile uint32_t msg_bit = 0;                                //To temporarily store the data received

volatile uint32_t new_key = 0;                                //To store the complete data recieved.


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
          ISRs
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
   This ISR counts the number of time elasped between two consecutive
   HIGH to LOW transition of the pulse.
*/
ISR(TIMER0_COMPA_vect) {
  if (count < 50) {
    ++count;
  }
}


/*
   This ISR is invoked whenever pulse goes from HIGH to LOW
*/
ISR(INT0_vect) {
  timer_value = count;
  count = 0;
  TCNT0 = 0;
  if (sof) {
    ++pulse_count;
  }

  if (timer_value >= 13 && timer_value <= 15) {
    msg_bit = 0;
    sof = 1;
    pulse_count = -1;
  }
  else if ((pulse_count >= 0) && (pulse_count <= 31)) {
    if (timer_value >= 2) {
      msg_bit |= (uint32_t)1 << (31 - pulse_count);
    }
    else {
    }
    if (pulse_count == 31) {
      EICRA = (1 << ISC00);
    }
  }
  else if (pulse_count >= 32)
  {
    new_key = msg_bit;
    pulse_count = -1;
    sof = 0;
    EICRA = (1 << ISC01);
  }
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
      OTHER FUNCTIONS
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
    TO ESTABLSIH USART COMMUNICATION AT 9600 BAUD RATE
 */
void usart_init(void)
{
  UCSR0A = 0x00;
  UCSR0B = (1 << RXEN0) | (1 << TXEN0);
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
  UBRR0L = 103;
}

/*
  TO PRINT DATA TO SERIAL MONITOR
*/
void USARTWriteChar(char data)
{
  while (!(UCSR0A & (1 << UDRE0)))
  {

  }
  UDR0 = data;
}


/*
   This fuction enable the external interrupt INT0
*/
void setup_external_interrupt() {
  EICRA = (1 << ISC01);
  EIMSK = (1 << INT0);
}


/*
   Ths function initiallises Timer0
*/
void init_timer0() {
  TCCR0A = (1 << WGM01);
  TIMSK0 = (1 << OCIE0A);
  OCR0A = 250;
}


/*
   This function starts the TIMER0
*/
void timer0_start() {
  TCCR0B = (1 << CS01) | (1 << CS00);
  TCNT0 = 0;
}

//Function to stop TIMER0
void timer0_stop() {
  TCCR0B = 0x00;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
      MAIN FUNCTION
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main() {

  DDRD = 0x00;                                //PORTD Input.
  
  usart_init();                               // Initiallise USART
  
  setup_external_interrupt();                 //Enable external interrupt INT0

  init_timer0();                              //Initiallise Timer0
  
  timer0_start();                             //Start Timer0
  
  sei();                                      //Enable Global interrupts

  while (1) {
    if (new_key) {
      if ( new_key == 0xB24DF20D ) {                              //Left
        USARTWriteChar('0');
        USARTWriteChar('\n');
      }
      else if ( new_key == 0xB24D629D ) {                         //Right
        USARTWriteChar('1');
        USARTWriteChar('\n');
      }
      else if ( new_key == 0xB24D56A9 ) {                         //Forward and Left
        USARTWriteChar('2');
        USARTWriteChar('\n');
      }
      else if ( new_key == 0xB24D52AD ) {                         //Forward and Right
        USARTWriteChar('3');
        USARTWriteChar('\n');
      }
      else if ( new_key == 0xB24D6699 ) {                         //Forward
        USARTWriteChar('4');
        USARTWriteChar('\n');
      }
      else if ( new_key == 0xB24DFF00 ) {                         //Stable
        USARTWriteChar('5');
        USARTWriteChar('\n');
      }
      new_key = 0;
    }

  }
  return 0;
}



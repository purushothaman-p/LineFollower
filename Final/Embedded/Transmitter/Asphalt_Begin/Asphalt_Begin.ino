/*
   The following program reads the accelerometer value(X axis and Y axis only) of sensor ADXL335
   and executes the corresponding command i.e to emit an IR signal according to gesture.
   IR signal generated follows NEC protocol.

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
         BRAKE: 00001111

       Hexadecimal Values
         LEFT: B24DF20D
         RIGHT: B24D629D
         FOR_LEFT: B24D56A9
         FOR_RIGHT: B24D52AD
         FORWARD: B24D6699
         STABLE: B24DFF00
         BRAKE: B24DF0F0
*/

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
        HEADER FILES
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
#include<avr/io.h>
#include<avr/interrupt.h>

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
       GLOBAL VARIABLES
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
   Various codes to execute after a gesture is detected
*/
uint32_t code_LEFT = 0b10110010010011011111001000001101;
uint32_t code_RIGHT = 0b10110010010011010110001010011101;
uint32_t code_FOR_LEFT = 0b10110010010011010101011010101001;
uint32_t code_FOR_RIGHT = 0b10110010010011010101001010101101;
uint32_t code_SHOOT = 0b10110010010011010110011010011001;
uint32_t code_REST = 0b10110010010011011111111100000000;
uint32_t code_BRAKE = 0b10110010010011011111000011110000;
uint32_t code = 0;

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
          ISRs
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

/*
   This routine will toggle the IR LED pin ON/OFF
*/
ISR(TIMER0_COMPA_vect) {
  PORTD ^= (1 << 2);
}

/*
   This routuine will stop the current job being executed when transmitting the IR signal.
*/
ISR(TIMER1_COMPA_vect) {
  TCCR1B = 0x00;
  TCCR0B = 0x00;
  PORTD = 0x00;
}

/*
   This routine will cause delay/keep an eye on Time out time.
*/
ISR(TIMER2_OVF_vect) {
  TCCR2B = 0x00;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
      TRANSMITTER FUNCTIONS
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/*
  Function to send Start pulse.
  Pulse is 9ms long burst of 38kHz IR light.
*/
void start_pulse() {
  OCR1A = 18000;
  TCCR1B = (1 << CS11);
  TCCR0B = (1 << CS00);
  TCNT1 = 0;
  TCNT0 = 0;
  PORTD = 0b00000100;
  while (TCCR1B & (1 << CS11));
}

/*
   Function to cause delay.
   Pulse is retained low for 4.5ms.
*/
void start_delay() {
  PORTD = 0x00;
  OCR1A = 9000;
  TCCR1B = (1 << CS11);
  TCNT1 = 0;
  while (TCCR1B & (1 << CS11));
}

/*
   Function to send 562.5us burst of IR signal showing start of a bit.
*/
void code_pulse() {
  OCR1A = 9000;
  TCCR1B = (1 << CS10);
  TCCR0B = (1 << CS00);
  TCNT1 = 0;
  TCNT0 = 0;
  PORTD = 0b00000100;
  while (TCCR1B & (1 << CS10));
}

/*
   Function to retain signal line low as per the time passed in time_tic.
   If bit is 1, it is retained low for 1687.5us
   If bit is 0, it is retained low for 562.5us
*/
void delay_bit(int time_tic) {
  PORTD = 0x00;
  OCR1A = time_tic;
  TCCR1B = (1 << CS10);
  TCNT1 = 0;
  while (TCCR1B & (1 << CS10));
}

/*
  Funtion to cause a delay in the program.
*/
void delay_time() {
  PORTD = 0x00;
  TCCR2B = (1 << CS22) | (1 << CS21) | (1 << CS20);                         //Timer2 Normal mode, 1024 Prescaler
  TCNT2 = 0;
  while (TCCR2B & (1 << CS20));
}

/*
   Function to send code according to the arguement passed.
*/
void send_code(int code_value) {
  if (code_value == 0) {
    code = code_LEFT;
  }
  else if (code_value == 1) {
    code = code_RIGHT;
  }
  else if (code_value == 2) {
    code = code_FOR_LEFT;
  }
  else if (code_value == 3) {
    code = code_FOR_RIGHT;
  }
  else if (code_value == 4) {
    code = code_SHOOT;
  }
  else if (code_value == 5) {
    code = code_REST;
  }
  start_pulse();
  start_delay();
  for (int i = 0; i < 32; ++i) {
    code_pulse();
    if (code & 0x80000000) {
      delay_bit(27000);
    }
    else {
      delay_bit(9000);
    }
    code <<= 1;
  }
  code_pulse();
  code = 0;
}


/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
      ADC FUNCTIONS
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/* 
  ADC Initialization function 
 */
void ADC_Init()     
{
  ADMUX = (1 << REFS0) | (1 << ADLAR); /* Vref: Avcc, ADC channel: 0 */
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

/* 
  ADC Read function 
*/
unsigned char ADC_Read(unsigned char channel)  
{
  ADMUX = (1 << REFS0) | (1 << ADLAR) | (channel & 0x07); /* set input channel to read */
  ADCSRA |= (1 << ADSC); /* Start ADC conversion */
  while (!(ADCSRA & (1 << ADIF))); /* Wait until end of conversion by polling ADC interrupt flag */
  ADCSRA |= (1 << ADIF); /* Clear interrupt flag */
  return ADCH;    /* Return ADC word */
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~
      MAIN FUNCTION
  ~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
int main() {

  int X, Y, Z;                                               //Variable to store the accelerometer values
  
  DDRB = 0xFF;                                               //PORTB OUTPUT
  
  Serial.begin(9600);                                        //Initiallising USART communication
  ADC_Init();   /* Initialize ADC */

  TCCR0A = (1 << WGM01);                                     //Timer0 CTC mode
  TCCR1A = (1 << WGM12);                                     //Timer1 CTC mode
  //Enable interrupts on compare match vect A for Timer 1 and 0
  TIMSK0 = (1 << OCIE0A);
  TIMSK1 = (1 << OCIE1A);
  //Enable interrupts on Timer Overflow, TIMER 2
  TIMSK2 = (1 << TOIE2);
  
  OCR0A = 210;
  
  sei();                                                     //Enable global interrupts.
  
  DDRD = 0b00000100;                                         //IR Led is connceted to PIN 2(PORT D).
  
  PORTD = 0x00;                                              //LED Off
  
  while (1) {
     /* 
      Read X, Y, Z axis ADC value and calibrate it in such a way that in stable position readings are 0. 
     */
    X = ADC_Read(0) - 83; 
    Y = ADC_Read(1) - 83;
    Z = ADC_Read(2) - 77;

    if (X < -4 && Y < 4) {
      send_code(0);
      Serial.print(0);
    }
    else if (X > 4 && Y < 4) {
      send_code(1);
      Serial.print(1);
    }
    else if (X < -4 && Y >= 4) {
      send_code(2);
      Serial.print(2);
    }
    else if (X > 4 && Y >= 4) {
      send_code(3);
      Serial.print(3);
    }
    else if (Y > 4) {
      send_code(4);
      Serial.print(4);
    }
    else if (X <= 4 & Y <= 4 & X >= -4 & Y >= -4) {
      send_code(5);
      Serial.print(5);
    }
    else if (Y <= -3 & X >= -4 & X <= 4) {
      send_code(6);
      Serial.print(6);
    }
    else {
      Serial.print(6); Serial.print('\t');
      Serial.print(X); Serial.print('\t'); Serial.println(Y);
    }
    delay_time();
  }
  PORTD = 0x00;
  return 0;
}


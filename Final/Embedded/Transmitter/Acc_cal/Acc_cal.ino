#include <avr/io.h>   /* Include AVR std. library file */
#include <util/delay.h>   /* Include defined delay header file */

void ADC_Init()     /* ADC Initialization function */
{
  ADMUX =(1<<ADLAR);   /* Vref: Avcc, ADC channel: 0 */
  ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

uint16_t ADC_Read(unsigned char channel)  /* ADC Read function */
{
  ADMUX =(1<<ADLAR)|(channel & 0x07);/* set input channel to read */
  ADCSRA |= (1<<ADSC);  /* Start ADC conversion */
  while (!(ADCSRA & (1<<ADIF)));  /* Wait until end of conversion by polling ADC interrupt flag */
  ADCSRA |= (1<<ADIF);  /* Clear interrupt flag */
  return ADC;    /* Return ADC word */
}
int main(void)
{
  uint16_t x,y,z;
  DDRB=0xFF;
  Serial.begin(9600);
  ADC_Init();   /* Initialize ADC */
  int flag=0;
  while(1)
  {
    x= ADC_Read(0)/100;  /* Read X, Y, Z axis ADC value */
    y= ADC_Read(1)/100;
    z= ADC_Read(2)/100;
    if(z>=216 && z<=252 && y>=210 && y<=229) {
      Serial.println("Stable");
    }
    else if(z<216 && y>=210 && y<=229) {
      Serial.println("Left");
    }
    else if(z>252 && y>=210 && y<=229) {
      Serial.println("Right");
    }
    else if(z>=216 && z<=252 && y>229 && y<=250) {
      Serial.println("Accelerate");
    }
    else if(z>=216 && z<=252 && y<210) {
      Serial.println("Brake");
    }
    else if(z<216 && y>229) {
      Serial.println("Acc and Left");
    }
    else if(z>252 && y>229) {
      Serial.println("Acc and Right");
    }
    else if(z>=216 && z<=252 && y>250) {
      Serial.println("Nitro");
    }
    else{
      Serial.println("Out");
    }
    /*Serial.print("X: ");
    Serial.print(x);Serial.print('\t');
    Serial.print("Y: ");
    Serial.print(y);Serial.print('\t');
    Serial.print("Z:");
    Serial.println(z);*/
    _delay_ms(10);
  }
}

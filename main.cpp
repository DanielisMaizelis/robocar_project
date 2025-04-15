#define F_CPU 16000000UL
#define BAUD 9600 // Define baud rate
#define MYUBRR F_CPU / 16 / BAUD - 1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

unsigned char data;
unsigned int IrLF = 0;
unsigned int IrRF = 0;
unsigned int IrL = 0;
unsigned int IrM = 0;
unsigned int IrR = 0;
uint8_t currentChannel = 1;
uint16_t countUltra = 0;
uint16_t tim2ovrFl = 0;

/*  Function prototypes  */
void go(float speedLeft, float speedRight);
void tim0PwmSetup();
void USART_Transmit(unsigned char data);
void USART_Init(unsigned int ubrr);
unsigned char USART_Receive(void);
void printString(unsigned char *str);
void printInt(int value);
void timer2Init();
void adcInit();
void runSecond(int left, int right);
void pinSetup();
void race();
void testIrSensor();
void testUltraSonicDistance();
void testUltraSonicStop();
/*  Function prototypes end*/

ISR(ADC_vect)
{
  uint8_t low = ADCL;
  uint8_t high = ADCH;
  int result = (high << 8) | low;

  switch (currentChannel)
  {
  case 1: // channel 1 (PC1)
    IrRF = result;
    currentChannel = 2;
    ADMUX = (ADMUX & 0xF0) | 0x01; // Select ADC1 (PC1)
    break;

  case 2: // channel 2 (PC2)
    IrR = result;
    currentChannel = 4;
    ADMUX = (ADMUX & 0xF0) | 0x02; // Select ADC2 (PC2)
    break;

  case 4: // channel 4 (PC4)
    IrL = result;
    currentChannel = 3;
    ADMUX = (ADMUX & 0xF0) | 0x04; // Select ADC4 (PC4)
    break;

  case 3: // channel 3 (PC3)
    IrM = result;
    currentChannel = 0;
    ADMUX = (ADMUX & 0xF0) | 0x03; // Select ADC3 (PC3)
    break;

  case 0: // channel 0 (PC0)
    IrLF = result;
    currentChannel = 1;
    ADMUX = (ADMUX & 0xF0); // Select ADC0 (PC0)
    break;
  }

  ADCSRA |= (1 << ADSC); // Start next conversion
}

ISR(TIMER2_OVF_vect)
{
  tim2ovrFl++;
  if (tim2ovrFl == 4) // Goes every 50 ms
  {
    countUltra = 0;
    PORTB &= ~(1 << PORTB4);
    _delay_us(2);
    PORTB |= (1 << PORTB4);
    _delay_us(10);
    PORTB &= ~(1 << PORTB4);
    while (!(PINB & (1 << PORTB5)));
    while (PINB & (1 << PORTB5))
    {
      countUltra++;
      _delay_us(1);
    }
    tim2ovrFl = 0;
  }
}

ISR(USART_RX_vect) 
{ 
  data = UDR0; 
}

int main(void)
{
  pinSetup();
  tim0PwmSetup();
  USART_Init(MYUBRR);
  adcInit();
  timer2Init();
  sei();
  while (1)
  {
    // Here I should make the readings of the sensor and send info to the termnial
  }
}

void race()
{
  if (data == 'a')
  {
    if ((countUltra / 58) <= 17) go(0, 0);
    else
    {
      if (IrRF > 840 || IrR > 660) go(0, 0.9); // Turn right
      else if (IrLF < 720 || IrL < 660) go(0.9, 0); // Turn left
      else go(0.7, 0.7); // Straight
#if 0
        // Previous code 
        if (IrRF > 840 && IrR > 660)     go(0,0.9);  // Turn right            
        else if (IrLF < 710 && IrL < 660) go(0.9,0); // Turn left             
        else                             go(0.7,0.7);   // Straight
#endif
  }
  }
  else if (data == 'b') go(0, 0);
  else if (data == 'c')
  {
    go(0, 0);
    _delay_ms(5000);
    data = 'a';
  }
}

void go(float speedLeft, float speedRight)
{
  OCR0B = 255 * speedLeft; // D5
  PORTD &= ~(1 << PORTD2);
  OCR0A = 255 * speedRight; // D6
  PORTD &= ~(1 << PORTD4);
}


/* Testing */
void testIrSensor()
{
  printInt(IrLF);
  USART_Transmit('\t');
  printInt(IrL);
  USART_Transmit('\t');
  printInt(IrM);
  USART_Transmit('\t');
  printInt(IrR);
  USART_Transmit('\t');
  printInt(IrRF);
  USART_Transmit('\n');
}

void testUltraSonicStop()
{
  // Stops after 17 cm
  if ((countUltra / 58) <= 17) go(0, 0);
  else
  {
    if (IrRF > 840 && IrR > 660) go(0, 0.9); // Turn right
    else if (IrLF < 710 && IrL < 660) go(0.9, 0); // Turn left
    else go(0.7, 0.7); // Straight
  }
}

void testUltraSonicDistance()
{
  printInt(countUltra / 58);
  USART_Transmit('\n');
}
void testRunSecond(int left, int right)
{
  go(0, 0);
  go(left, right);
  _delay_ms(1000);
  go(0, 0);
  go(left * 0.7, right * 0.7);
  _delay_ms(1000);
}
/* Testing - end */

/* Setup */
void pinSetup()
{
  DDRD = (1 << PORTD5) | (1 << PORTD2) | (1 << PORTD6) | (1 << PORTD4);
  DDRB |= (1 << PORTB4);
  DDRB &= ~(1 << PORTB5);
}

void tim0PwmSetup()
{
  TCCR0A |= (1 << WGM01) | (1 << WGM00);
  TCCR0B |= (1 << CS00);
  TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
}

void timer2Init()
{
  TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20); // Sets prescalar to
  TIMSK2 |= (1 << TOIE2);                            // Enables the interrupt
}
void adcInit()
{
  ADMUX = (1 << REFS0) | (1 << MUX0); // Start with adcLeftFast (PC1)
  ADCSRA = (1 << ADEN) | (1 << ADSC) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}
/* Setup end */

/* Terminal print */
void printInt(int value)
{
  if (value == 0)
  {
    USART_Transmit('0');
    return;
  }
  unsigned char intStr[10];
  int count = 0;
  while (value > 0)
  {
    intStr[count++] = '0' + (value % 10);
    value /= 10;
  }
  // Print the number in the correct order
  for (int i = count - 1; i >= 0; i--)
    USART_Transmit(intStr[i]);
}

void printString(unsigned char *str)
{
  while (*str) USART_Transmit(*str++);
}

void USART_Init(unsigned int ubrr)
{
  /*Set baud rate */
  UBRR0H = (unsigned char)(ubrr >> 8);
  UBRR0L = (unsigned char)ubrr;
  /*Enable receiver and transmitter */
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0) | (1 << RXEN0);
  /* Set frame format: 8data, 1stop bit */
  UCSR0C = (3 << UCSZ00);
}

void USART_Transmit(unsigned char data)
{
  UDR0 = data;
}
/* Terminal print end*/

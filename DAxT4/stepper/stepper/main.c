/*
 * DAX4.c
 * STEPPER CONTROL
 *
 * Created: 4/10/2018 9:08:42 PM
 * Author : Kyle
 */
#define F_CPU 8000000UL
#include <avr/io.h> 
#include <util/delay.h> 
#include <avr/interrupt.h>
#include <string.h> 
#define BAUDRATE 9600 
#define BAUD_PRESCALER (((F_CPU/(BAUDRATE*16UL)))-1) 
 
static int count;
 
void USART_INIT(void) 
{ 
    // This function sets up the microcontroller for serial communication
    // The baud rate, registers, and prescaler value are all set.
    UBRR0H = (uint8_t)(BAUD_PRESCALER>>8); 
    UBRR0L = (uint8_t)(BAUD_PRESCALER); 
    UCSR0B = (1<<RXEN0)|(1<<TXEN0); 
    UCSR0C = (3<<UCSZ00);  
} 
 
void USART_SEND(unsigned char data)  
{ 
    // This function sends a single character from the ATmega328P to the computer.
    while(!(UCSR0A&(1<<UDRE0))); 
    UDR0=data; 
} 
 
 
uint16_t ReadADC(uint8_t ADCchannel) 
{ 
    // This function reads and returns the ADC value from a specified input pin
    ADMUX = (ADMUX&0xF0)|(ADCchannel&0x0F); // select ADC channel with safety mask
    ADCSRA |= (1<<ADSC);  // single conversion mode
    while( ADCSRA & (1<<ADSC));// wait until ADC conversion is complete
    return ADC; 
}

int main(void) 
{
    // set up timer with prescaler = 64 and CTC mode
    count = 0;
    DDRB = 0xFF;
    ADMUX |= (1<<REFS0); // select Vref=AVcc
    ADCSRA |= (1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);   // set prescaler to 8 and enable ADC
    USART_INIT();  // Call the USART initialization code
    TCCR1B |= (1 << WGM12)|(1 << CS11)|(1 << CS10);
    TCNT1 = 0;
    while(1) 
    { 
        int temp = ReadADC(0)*10;// read the ADC value and perform conversion to temperature
        OCR1A = temp;
     
        if(TIFR1 & (1<<OCF1A)){
            PORTB = 0x66; //PB1 PB2
            _delay_ms(10);
            PORTB = 0xCC;  //PB2 PB3
            _delay_ms(10);
            PORTB = 0x99;   //PB0 PB3
            _delay_ms(10);
            PORTB = 0x33;  // PB1 PB0
            _delay_ms(10);
             
             
            USART_SEND((temp/10000)%10+48); // send thousand place digit as a character
            USART_SEND((temp/1000)%10+48);  // send huns place digit as a character
            USART_SEND((temp/100)%10+48);   // send tens place digit as a character
            USART_SEND((temp/10)%10+48);    // send ones place digit as a character
            USART_SEND(46);                 // send a decimal point: "."
            USART_SEND(temp%10+48);         // send tenths place digit as a character
            USART_SEND(10);             // send a newline
             
        }
            TIFR1 |= (1<<OCF1A);
             
    } 
    return 0; 
}
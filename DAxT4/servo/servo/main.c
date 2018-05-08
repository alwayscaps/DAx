/*
 * servo.c
 *
 * Created: 4/10/2018 10:42:21 PM
 *  Author: Kyle
 */ 
#define F_CPU 8000000
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#define BAUDRATE 9600
#define BAUD_PRESCALER (((F_CPU/(BAUDRATE*16UL)))-1)

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

void main()
{
	TCCR1A |= (1<<COM1A1)|(1<<COM1B1)|(1<<WGM11); //setting mode
	
	TCCR1B |= (1<<WGM13)|(1<<WGM12)|(1<<CS11); //prescale 8
	
	ICR1 = 19999;  //50hz
	
	DDRB = 0xFF;
	ADMUX |= (1<<REFS0); // select Vref=AVcc
	ADCSRA |= (1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);   // set prescaler to 8 and enable ADC
	USART_INIT();
	while(1)
	{
		int temp = ReadADC(0)*2.5;			//read pot value and scale up
			USART_SEND((temp/100)%10+48);	// send tens place digit as a character
			USART_SEND((temp/10)%10+48);	// send ones place digit as a character
			USART_SEND(46);					// send a decimal point: "."
			USART_SEND(temp%10+48);			// send tenths place digit as a character
			//_delay_ms(100);
			USART_SEND(10);
		if(temp <750)						//keeping in range of servo degrees to save plasic gear
			temp = 750;
		if(temp > 2275)
		temp = 2275;
		
		OCR1A = temp; //0 degree = 750// 180 degree = 2275
		_delay_ms(100);
		
	}
}
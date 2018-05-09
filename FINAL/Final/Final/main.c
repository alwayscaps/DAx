/*
 * PID_Speed_Control.c
 *
 * Created: 5/1/2018 8:32:57 PM
 * Author : Kyle
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>

#define BAUDRATE 9600
#define BAUD_PRESCALER (((F_CPU/(BAUDRATE*16UL)))-1)
#define F_CPU 8000000UL

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

unsigned char USART_RECEIVE(void)
{
	while(!(UCSR0A & (1<<RXC0)));
	return UDR0;
}

int main(void)
{

	DDRD |= (1<<PD6) | (1<<PD1) ;
	DDRD &= ~(1<<PD2);
	PORTD |= (1<<PD2); // pull up resistor
	

	DDRB=0xFF; // portB is output for pwm.
	
	TCCR0A |= (1<<COM0A1)|(1<<WGM00)|(WGM01);  //set pwm fast
	TCCR0B |= (1<<CS00); //Configure TCCR0 as explained in the article
	
	unsigned char promptSpeed[35] = "This is the Current Speed: ## RPM\n";
	unsigned char desiredSpeed[41] = "How Fast Would you like the motor to go? ";
	unsigned char desiredSpeedNum[3];
	USART_INIT();

	OCR0A=100; // Set OCR0 to 255 for full speed and 0 for stop


	// Tell user the current speed
	for(int i=0; i< 35; i++)
	{
		USART_SEND(promptSpeed[i]);
		_delay_ms(10);
	}
	USART_SEND(10); // send line feed
	
	// Ask user for speed
	for(int i=0; i< 41; i++)
	{
		USART_SEND(desiredSpeed[i]);
		_delay_ms(10);
	} 
	
	// get the desired speed from user
	desiredSpeedNum[0] = USART_RECEIVE();
	USART_SEND(desiredSpeedNum[0]);
	desiredSpeedNum[1] = USART_RECEIVE();
	USART_SEND(desiredSpeedNum[1]);
	desiredSpeedNum[2] = USART_RECEIVE();
	USART_SEND(desiredSpeedNum[2]);	
	USART_SEND(10);	
	
	//set the desired speed
	int DSpeed = ((desiredSpeedNum[0]-48)*100) + ((desiredSpeedNum[1]-48)*10) + ((desiredSpeedNum[2]-48));
	OCR0A = DSpeed;
	
    while (1) 
    {
		if( !(PIND & (1<<PD2)) ) // if the button was pushed
		{
			while(OCR0A > 50)
			{
				OCR0A -= 10;		// slowly decrease the speed of the motor
				_delay_ms(1000);
			}			// when the motor speed < 50, then turn the motor off
			OCR0A = 0;
		}
    }
}
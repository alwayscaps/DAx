#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <string.h>
#include <util/delay.h>
#include "nrf24l01.h"
#define F_CPU 8000000UL

#define BAUDRATE 9600
#define BAUD_PRESCALER (((F_CPU/(BAUDRATE*16UL)))-1)

nRF24L01 *setup_rf(void);
void process_message(char *message);
inline void prepare_led_pin(void);
inline void  set_led_high(void);
inline void  set_led_low(void);

volatile bool rf_interrupt = false;

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

int main(void) {
	uint8_t address[5] = { 0x01, 0x01, 0x01, 0x01, 0x01 };
	prepare_led_pin();
	sei();
	nRF24L01 *rf = setup_rf();
	nRF24L01_listen(rf, 0, address);
	uint8_t addr[5];
	uint8_t config = 0x00;
	nRF24L01_read_register(rf, config, addr, 1);
	USART_INIT();

	while (true) {
		if (rf_interrupt) {
			rf_interrupt = false;
			while (nRF24L01_data_received(rf)) {
				nRF24L01Message msg;
				nRF24L01_read_received_data(rf, &msg);
				set_led_high();
				for(int i=0; i < 2; i++)
				{
					USART_SEND(msg.data[i]);
				}
				USART_SEND(10);
				nRF24L01_flush_receive_message(rf);
			}

			nRF24L01_listen(rf, 0, address);
		}
		set_led_low();
	}

	return 0;
}

nRF24L01 *setup_rf(void) {
	nRF24L01 *rf = nRF24L01_init();
	rf->ss.port = &PORTB;
	rf->ss.pin = PB2;
	rf->ce.port = &PORTB;
	rf->ce.pin = PB1;
	rf->sck.port = &PORTB;
	rf->sck.pin = PB5;
	rf->mosi.port = &PORTB;
	rf->mosi.pin = PB3;
	rf->miso.port = &PORTB;
	rf->miso.pin = PB4;
	// interrupt on falling edge of INT0 (PD2)
	EICRA |= _BV(ISC01);
	EIMSK |= _BV(INT0);
	nRF24L01_begin(rf);
	return rf;
}

void process_message(char *message) {
	if (strcmp(message, "ON") == 0)
	set_led_high();
	else if (strcmp(message, "OFF") == 0)
	set_led_low();
}

inline void prepare_led_pin(void) {
	DDRB |= _BV(PB0);
	PORTB &= ~_BV(PB0);
}

inline void set_led_high(void) {
	PORTB |= _BV(PB0);
}

inline void set_led_low(void) {
	PORTB &= ~_BV(PB0);
}

// nRF24L01 interrupt
ISR(INT0_vect) {
	rf_interrupt = true;
}

/*Transmit 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <string.h>
#include "nrf24l01.h"

#define F_CPU 8000000

void setup_timer(void);
nRF24L01 *setup_rf(void);
inline void prepare_led_pin(void);
inline void  set_led_high(void);
inline void  set_led_low(void);

volatile bool rf_interrupt = false;
volatile bool send_message = false;

uint16_t ReadADC(uint8_t ADCchannel)
{
	// This function reads and returns the ADC value from a specified input pin
	ADMUX = (ADMUX&0xF0)|(ADCchannel&0x0F); // select ADC channel with safety mask
	ADCSRA |= (1<<ADSC);  // single conversion mode
	while( ADCSRA & (1<<ADSC));// wait until ADC conversion is complete
	return ADC;
}

void ADC_Init()
{
	ADMUX |= (1<<REFS0); // select Vref=AVcc
	ADCSRA |= (1<<ADPS1)|(1<<ADPS0)|(1<<ADEN);   // set prescaler to 8 and enable ADC
}


int main(void) {
	uint8_t to_address[5] = { 0x01, 0x01, 0x01, 0x01, 0x01 };
	ADC_Init();
	int temp;
	sei();
	nRF24L01 *rf = setup_rf();
	setup_timer();
	prepare_led_pin();

	while (true) {
		if (rf_interrupt) {
			rf_interrupt = false;
			int success = nRF24L01_transmit_success(rf);
			if (success != 0)
			nRF24L01_flush_transmit_message(rf);
			set_led_low();
		}

		if (send_message) {
			set_led_high();
			send_message = false;
			temp=ReadADC(0)*6+320;
			nRF24L01Message msg;
			msg.data[0] = (char)(temp/10)%10+48;
			msg.data[1] = (char)(temp/100)%10+48;
			msg.length = strlen((char *)msg.data) + 1;
			nRF24L01_transmit(rf, to_address, &msg);
		}
	}

	return 0;
}

nRF24L01 *setup_rf(void) {
	nRF24L01 *rf = nRF24L01_init();
	rf->ss.port = &PORTB;
	rf->ss.pin = PB2;
	rf->ce.port = &PORTB;
	rf->ce.pin = PB1;
	rf->sck.port = &PORTB;
	rf->sck.pin = PB5;
	rf->mosi.port = &PORTB;
	rf->mosi.pin = PB3;
	rf->miso.port = &PORTB;
	rf->miso.pin = PB4;
	// interrupt on falling edge of INT0 (PD2)
	EICRA |= _BV(ISC01);
	EIMSK |= _BV(INT0);
	nRF24L01_begin(rf);
	return rf;
}

// setup timer to trigger interrupt every second when at 1MHz
void setup_timer(void) {
	TCCR1B |= _BV(WGM12);
	TIMSK1 |= _BV(OCIE1A);
	OCR1A = 15624;
	TCCR1B |= _BV(CS10) | _BV(CS11);
}

// each one second interrupt
ISR(TIMER1_COMPA_vect) {
	send_message = true;
}

// nRF24L01 interrupt
ISR(INT0_vect) {
	rf_interrupt = true;
}

inline void prepare_led_pin(void) {
	DDRB |= _BV(PB0);
	PORTB &= ~_BV(PB0);
}

inline void set_led_high(void) {
	PORTB |= _BV(PB0);
}

inline void set_led_low(void) {
	PORTB &= ~_BV(PB0);
}

*/
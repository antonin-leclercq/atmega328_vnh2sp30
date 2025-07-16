/*
 * atmega328_vnh2sp30_demo.c
 *
 * Created: 27/01/2023 13:53:42
 * Author : LECLERCQ Antonin
 */ 

#define F_CPU 16000000
#define BAUD 9600

#include <avr/io.h>
#include <util/setbaud.h>
#include <avr/interrupt.h>

extern int avr_printf(const char *format, ...);
extern int avr_sprintf(char *out, const char *format, ...);

#define PWM PINB1
#define INA PIND2
#define INB PIND4
#define ENA PIND6

#define TOP_VALUE 200
#define PWM_STEP 10

#define RXD PIND0
#define TXD PIND1

char received_byte;

const char* user_message = "VNH2SP30 controller:\r\n" \
							"\t-press l to go left and r to go right\r\n" \
							"\t-press p to increase duty cycle and m to decrease duty cycle\r\n" \
							"\t-press s to stop the motor\r\n";
							
uint16_t pwm_val = 0;
							
typedef enum {
	LEFT,
	RIGHT,
	INCREASE,
	DECREASE,
	STOP,
	DEFAULT
} RECEIVE_TYPE ;

volatile RECEIVE_TYPE rcv = DEFAULT;

void Init_VNH2SP30(void) {
	/*
	Initialize GPIO
	PWM --> PB1 (on OC1A)
	INA --> PD2 (output)
	INB --> PD4 (output)
	ENA --> PD6 (input)
	Pull-up resistors are needed for ENA (open drain)s
	Max PWM frequency for the VNH2SP30 is 10kHz
	*/
	
	DDRB |= (1 << PWM); // Set as output
	DDRD |= (1 << INA) | (1 << INB); // Set as outputs
	DDRD &= ~(1 << ENA); // Set as inputs
	
	// Add pullup-resistors for ENA
	PORTD |= (1 << ENA);
	
	// Timer 1: Set to Fast PWM with ICR1 as TOP value
	TCCR1A |= (1 << WGM11); 
	TCCR1A &= ~(1 << WGM10);
	TCCR1B |= (1 << WGM13) | (1 << WGM12);
	
	// Clear OC1A/OC1B on Compare Match, set OC1A/OC1B at BOTTOM (non-inverting mode)
	TCCR1A |= (1 << COM1A1);
	TCCR1A &= ~(1 << COM1A0);
	
	// Set prescaler : /8 --> Counting frequency = 16MHz /8 = 2MHz (0.5us)
	TCCR1B |= (1 << CS11); 
	TCCR1B &= ~((1 << CS12) | (1 << CS10));
	
	// Set TOP value: 
	// for 10kHz, period is 100us, or 100us/0.5us = 200 counts ==> ICR1 = 200
	ICR1 = TOP_VALUE;
}

void Init_Serial(void) {
	/*
	Initialize GPIO:
	PD0 --> RXD (input, alternate function)
	PD1 --> TXD (output, alternate function)
	
	Asynchronous Normal Mode (8 data bits, 1 stop bit)
	BAUDRATE = 9600  <=>  UBRR = (Fosc / 16*BAUDRATE) - 1 = 16M / 16*9600 - 1 = 103
	
	With interrupts handling
	*/
	
	DDRD &= ~(1 << RXD); // Not necessary
	DDRD |= (1 << TXD); // Not necessary
	
	// Use asynchronous normal mode (default)
	UCSR0C &= ~((1 << UMSEL01) | (1 << UMSEL00));
	
	// Set UBRR for correct baudrate
	UBRR0H = UBRRH_VALUE;
	UBRR0L = UBRRL_VALUE;
	
	// Set to 8 data bit and 1 stop bit
	UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00);
	
	// Enable RX complete interrupt
	UCSR0B |= (1 << RXCIE0);
	
	// Enable Receiver and Transmitter
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0);
}

ISR(USART_RX_vect)
{
	// Store UDR0
	received_byte = (char)UDR0;
	
	switch(received_byte){
		case 'l':
		rcv = LEFT; break;
		case 'r':
		rcv = RIGHT; break;
		case 'p':
		rcv = INCREASE; break;
		case 'm':
		rcv = DECREASE; break;
		case 's':
		rcv = STOP; break;
		default:
		rcv = DEFAULT; break;
	}
}

int main(void)
{
    // Init Motor Driver
    Init_VNH2SP30();
	
	// Init Serial Communication
	Init_Serial();
	
	// Send user message
	avr_printf("%s", user_message);
	
	// Enable Global interrupts
	sei();
	
	PORTD &= ~((1 << INA) | (1 << INB));
	
	while (1) 
    {
		switch (rcv) {
			
			case LEFT:
			PORTD |= (1 << INA);
			PORTD &= ~(1 << INB);
			rcv = DEFAULT;
			avr_printf("%s\r\n", "Going Left");
			break;
			
			case RIGHT:
			PORTD |= (1 << INB);
			PORTD &= ~(1 << INA);
			rcv = DEFAULT;
			avr_printf("%s\r\n", "Going Right");
			break;
			
			case INCREASE:
			if (pwm_val <= TOP_VALUE - PWM_STEP) pwm_val += PWM_STEP;
			rcv = DEFAULT;
			avr_printf("%s\r\n", "Increasing duty cycle");
			break;
			
			case DECREASE:
			if (pwm_val >= PWM_STEP) pwm_val -= PWM_STEP;
			rcv = DEFAULT;
			avr_printf("%s\r\n", "Decreasing duty cycle");
			break;
			
			case STOP:
			PORTD &= ~((1 << INA) | (1 << INB));
			rcv = DEFAULT;
			avr_printf("%s\r\n", "Stopping...");
			break;	
				
			default:
			break;
		}
		
		// Update Compare register
		OCR1A = pwm_val;
    }
}


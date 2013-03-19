/*
 * car.c
 *
 * Created: 17.03.2013 11:39:55
 *  Author: Stas Dymedyuk
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define PIN_INPUT_TURN_LEFT PB0
#define PIN_INPUT_TURN_RIGHT PB1
#define PIN_INPUT_MOVE_FORWARD PB2
#define PIN_INPUT_MOVE_BACK PB3

#define PIN_TURN_LEFT PC0
#define PIN_TURN_RIGHT PC1
#define PIN_MOVE_BACK PC2
#define PIN_ON_OFF_ENGINE_POWER PC3

#define MAX_GEAR 5
#define MIN_GEAR 0

volatile unsigned int current_gear = 0;

volatile unsigned char gear_numbers[] = {
	0b00111111, // 0
	0b00000110, // 1
	0b01011011, // 2
	0b01001111, // 3
	0b01100110, // 4
	0b01101101, // 5
	0b01111101, // 6
	0b00000111, // 7
	0b01111111, // 8
	0b01101111  // 9
};


void setup_ports_settings();
void setup_interrups_settings();
void delay_ms(uint16_t count);
void show_gear_on_display();

int main(void) {
	setup_ports_settings();
	setup_interrups_settings();
	show_gear_on_display();
	
	unsigned int motors_step_period_in_ms = 100; 
	unsigned int motors_moved_period_in_ms = motors_step_period_in_ms / MAX_GEAR;
	unsigned int working_gear = current_gear;
						
    while(1) {		
		if ((PINB & (0x00 | 1<<PIN_INPUT_TURN_LEFT)) == 1<<PIN_INPUT_TURN_LEFT) {
			PORTC |= 1<<PIN_TURN_LEFT;
		} else {
			PORTC &= 0<<PIN_TURN_LEFT;
		}
		
		
		if ((PINB & (0x00 | 1<<PIN_INPUT_TURN_RIGHT)) == 1<<PIN_INPUT_TURN_RIGHT) {
			PORTC |= 1<<PIN_TURN_RIGHT;
		} else {
			PORTC &= 0<<PIN_TURN_RIGHT;
		}
		
		
		unsigned char car_moved_back = ((PINB & (0x00 | 1<<PIN_INPUT_MOVE_BACK)) == 1<<PIN_INPUT_MOVE_BACK);
		
		if ((PINB & (0x00 | 1<<PIN_INPUT_MOVE_FORWARD)) == 1<<PIN_INPUT_MOVE_FORWARD || car_moved_back == 1) {
			working_gear = current_gear;
		} else {
			working_gear = 0;
		}
		
		
		if (car_moved_back == 1) {
			PORTC |= 1<<PIN_MOVE_BACK;
		} else {
			PORTC &= 0<<PIN_MOVE_BACK;
		}
		
		
		PORTC |= 1<<PIN_ON_OFF_ENGINE_POWER;
		delay_ms(motors_moved_period_in_ms * working_gear);
		PORTC &= 0<<PIN_ON_OFF_ENGINE_POWER;
		delay_ms(motors_moved_period_in_ms * (MAX_GEAR - working_gear));		
		
    }
}


void setup_ports_settings() {
	DDRB = 0x00;
	DDRA = 0xFF;
	DDRC = 0xFF;
}

void setup_interrups_settings() {
	cli();
	GICR |= (1<<INT0) | (1<<INT1);
	MCUCR |= (1<<ISC00) | (1<<ISC01);
	MCUCR |= (1<<ISC10) | (1<<ISC11);	
	sei();	
}

void delay_ms(uint16_t count) {
	while(count--) {
		_delay_ms(1);
	}
}

void show_gear_on_display() {
	PORTA = gear_numbers[current_gear];
}

ISR(INT0_vect) {
	// up gear
	if (current_gear < MAX_GEAR) {
		current_gear++;
		show_gear_on_display();
	}
}

ISR(INT1_vect) {
	// down gear
	if (current_gear > MIN_GEAR) {
		current_gear--;
		show_gear_on_display();
	}
}

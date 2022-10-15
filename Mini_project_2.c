/*==========================================================================================================
Name: NADA Sayed Sayed
Project Name: Stop Watch using atmega32
===========================================================================================================*/
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

/* Global variables */
unsigned char unit_seconds=0; /*unit digit of seconds */
unsigned char tens_seconds=0; /*tens digit of seconds */
unsigned char unit_minutes=0; /*unit digit of minutes */
unsigned char tens_minutes=0; /*tens digit of minutes */
unsigned char unit_hours=0; /*unit digit of hours */
unsigned char tens_hours=0; /*tens digit of hours */

/* The interrupt service routine of Timer 1 Compare Mode which is called every second */
ISR(TIMER1_COMPA_vect){

	/*The number of seconds increase by one each call */
	unit_seconds++;

	/*Adapting the stop watch display each second */
	if(unit_seconds==10){
		unit_seconds=0;
		tens_seconds++;
	}
	if(tens_seconds==6){
		tens_seconds=0;
		unit_minutes++;
	}
	if(unit_minutes==10){
		unit_minutes=0;
		tens_minutes++;
	}
	if(tens_minutes==6){
		tens_minutes=0;
		unit_hours++;
	}
	if(unit_hours==10){
		unit_hours=0;
		tens_hours++;
	}
}

/* The interrupt service routine of Interrupt 0 */
ISR(INT0_vect){

	/* Setting all variables' values to zero to reset the stop watch */
	unit_seconds=0;
	tens_seconds=0;
	unit_minutes=0;
	tens_minutes=0;
	unit_hours=0;
	tens_hours=0;
}

/* The interrupt service routine of Interrupt 1 */
ISR(INT1_vect){

	/* Setting CS10 AND CS11 bits to zero to stop the timer clock (pause time) */
	TCCR1B &= ~(1<<CS10) & ~(1<<CS11);
}

/* The interrupt service routine of Interrupt 2 */
ISR(INT2_vect){

	/* Setting CS10 AND CS11 bits to one to start the timer clock (resume time) */
	TCCR1B |= (1<<CS10)|(1<<CS11);

}

void Timer1_Init(void){

	/* MC frequency = 1 MHz     Pre-scalar= 64
	 * Timer frequency = 1M/64 = 15625 Hz
	 * Timer count= 1/15625 seconds
	 * Number of counts needed for the timer to make one compare match each 1 second =
	 15625
	 */

	TCNT1 = 0;		/* Set timer1 initial count to zero */
	OCR1A = 15625;    /* Set the Compare Match value to 15625 */

	/* Configure timer control register TCCR1A
	 * 1. Disconnect OC1A and OC1B  COM1A1=0 COM1A0=0 COM1B0=0 COM1B1=0
	 * 2. FOC1A=1 FOC1B=0
	 * 3. CTC Mode WGM10=0 WGM11=0 (Mode Number 4)
	 */
	TCCR1A = (1<<FOC1A);

	/* Configure timer control register TCCR1B
	 * 1. CTC Mode WGM12=1 WGM13=0 (Mode Number 4)
	 * 2. Clock selection CS11=1 CS10=1 CS12=0
	 */
	TCCR1B = (1<<WGM12)|(1<<CS10)|(1<<CS11);

	TIMSK |= (1<<OCIE1A); /* Enable Timer1 Compare A Match Interrupt */

}

void Interrupt0_Init(void){

	/*Initializing Interrupt 0
	 * 1. Enabling interrupt 0 with falling edge ISC01=1 ISC00=0
	 * 2. Configure PD2 pin as input pin
	 * 3. Activating internal pull up at PD2 pin
	 * 4. Enabling interrupt request INT0 =1
	 */
	MCUCR|=(1<<ISC01);
	DDRD&=~(1<<PD2);
	PORTD|=(1<<PD2);
	GICR|=(1<<INT0);
}

void Interrupt1_Init(void){

	/*Initializing Interrupt 1
	 * 1. Enabling interrupt 1 with rising edge ISC11=1 ISC10=1
	 * 2. Configure PD2 pin as input pin
	 * 3. Enabling interrupt request INT1 =1
	 */
	MCUCR|=(1<<ISC10)|(1<<ISC11);
	DDRD&=~(1<<PD3);
	GICR|=(1<<INT1);
}

void Interrupt2_Init(void){

	/*Initializing Interrupt 2
	 * 1. Enabling interrupt 2 with falling edge ISC2=0
	 * 2. Configure PB2 pin as input pin
	 * 3. Activating internal pull up at PB2 pin
	 * 4. Enabling interrupt request INT2 =1
	 */
	MCUCSR &=~ (1<<ISC2);
	DDRB&=~(1<<PB2);
	PORTB|=(1<<PB2);
	GICR |=(1<<INT2);
}

int main(void){

	/* Variable carrying the number of multiplexed 7-segments */
	unsigned char number_of_segment;

	SREG |= (1<<7); /* Enable global interrupts in MC */

	DDRC |= 0x0F;   /* Configure the first 4 pins in Port C as output pins */
	PORTC &= 0xF0;  /* Initialize the 7-segment to display zero at the beginning */

	DDRA |= 0x3F;   /* Configure the first 6 pins of Port A as output pins */

	/* Initializing */
	Interrupt0_Init();
	Interrupt1_Init();
	Interrupt2_Init();
	Timer1_Init();

	while(1){

		/* Looping on the 6 7-segments using vision illusion technique
		    and enabling them 1 by 1*/
		for(number_of_segment=0;number_of_segment<6;number_of_segment++){

			/* Enabling the chosen 7-segment */
			PORTA=(PORTA & 0xC0)|(1<<number_of_segment);

			/*Displaying the time depending on which segment is enabled */
			if(number_of_segment==0){
				PORTC= (PORTC & 0XF0)|(unit_seconds & 0X0F);
			}
			else if(number_of_segment==1){
				PORTC= (PORTC & 0XF0)|(tens_seconds & 0X0F);
			}
			else if(number_of_segment==2){
				PORTC= (PORTC & 0XF0)|(unit_minutes & 0X0F);
			}
			else if(number_of_segment==3){
				PORTC= (PORTC & 0XF0)|(tens_minutes & 0X0F);
			}
			else if(number_of_segment==4){
				PORTC= (PORTC & 0XF0)|(unit_hours & 0X0F);
			}
			else if(number_of_segment==5){
				PORTC= (PORTC & 0XF0)|(tens_hours & 0X0F);
			}
			/* Waiting 5 milliseconds for vision illusion */
			_delay_ms(5);

			/* Disabling the chosen 7-segment to enable another in the next count */
			PORTA&=~(1<<number_of_segment);
		}
	}
}

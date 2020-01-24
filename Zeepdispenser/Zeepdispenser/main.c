/*
 * Zeepdispenser.c
 *
 * Created: 18/12/2019 01:50:09
 * Author : Jordy
 */ 

#define F_CPU 1200000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>


// Fuses: Low = 0x6A, High = 0xFF

//#define PIN_BUTTON  PB3
#define PIN_LED_E   PB0
#define PIN_LED_W   PB2
#define PIN_LED_S   PB3

// Teller variabele 
volatile unsigned long ulMil = 0;

void doSleep()
{
	// Interrupt aanzetten (knopje onder de beugel die de MCU wekt)
	GIMSK |= (1 << INT0);
	// Sleep mode instellen op 'Power down'
	MCUCR |= (1 << SM1);
	
	// Slaapfunctie aanzetten in MCUCR (Een slaapliedje zingen :P)
	sleep_enable();
	
	// Welterusten!
	sleep_cpu();
	//////////////////////////////////////////////////////////////////////////
	// CPU gaat hier verder als deze ontwaakt. Goeiemorgen!
	
	// Interrupt uitzetten
	GIMSK &= ~(1 << INT0);
	// Slaapfunctie uitzetten
	sleep_disable();
}

int main(void)
{
	// Variabelen declareren
	unsigned int uiStap = 0;
	unsigned long ulTimer = 0;
	
	// Enable LED Outputs
	DDRB |= (1 << PIN_LED_E) | (1 << PIN_LED_W) | (1 << PIN_LED_S);
	// Timer in CTC mode zetten
	TCCR0A |= (1 << WGM01);
	// Clock prescaler van CLKio/8 instellen
	TCCR0B |= (1 << CS01);
	
	// Interrupt van 1KHz (1000Hz) --> OCR = (CLKio / (2*prescaler*Focr)) - 1 --> OCR = 74
	// CLKio = 1,2 MHz (1200000 Hz), prescaler = 8, Focr = 1000
	// Maar, uitkomst * 2 (want we willen geen waveform genereren) --> OCR = 148
	// 
	OCR0A = 144;
	
	// Timer compare interrupt aanzetten om de 1KHz interrupt te starten
	TIMSK0 |= (1 << OCIE0A);
	
	// Alle (ingestelde) interrupts inschakelen
	sei();
	
	// CPU laten slapen
	doSleep();
	
	
	while(1)
	{
		
		// Stap bijhouden
		
		switch(uiStap)
		{
			// LED E aan
			case 0: // E
				PORTB |= (1 << PIN_LED_E);
				ulTimer = ulMil + 500;
				uiStap++;
				break;
			
			// Wachten op de timer 
			case 1:
				if(ulMil >= ulTimer) uiStap++;
				break;
			
			// LED E uit, W aan
			case 2: // W
				PORTB &= ~(1 << PIN_LED_E);
				PORTB |= (1 << PIN_LED_W);
				ulTimer = ulMil + 500;
				uiStap++;
				break;
			
			// Wachten op de timer
			case 3:
				if(ulMil >= ulTimer) uiStap++;
				break;
			
			// W uit, S aan
			case 4: // S
				PORTB &= ~(1 << PIN_LED_W);
				PORTB |= (1 << PIN_LED_S);
				ulTimer = ulMil + 500;
				uiStap++;
				break;
			
			// Wachten op de timer
			case 5:
				if(ulMil >= ulTimer) uiStap++;
				break;
			
			 // LEDs uit en wachten tot de beugel niet meer is bediend
			case 6:
				PORTB = 0;
				// Als de beugel niet meer bediend is
				if((PINB >> PB1) & 0x01) uiStap++;
				break;
		
			// MCU laten slapen
			case 7:
				doSleep();
				
				uiStap = 0;
				break;
			
			default:
				PORTB = 0;
				break;
		}
		
	}
	return 0;
}

// Teller om een vertragingssignaal te genereren.
// Hiermee kan elk lampje een bepaalde tijdsduur aangezet worden.
ISR(TIM0_COMPA_vect)
{
	ulMil++;
}

// Getriggerd op een laag niveau op PB1
ISR(INT0_vect)
{
	// Als deze getriggerd wordt, dan de microcontroller ontwaken
	// Dit gebeurt vanzelf, dus hier hoeft niets gedaan te worden.
}
// 2014 Roving Dynamics Ltd. Code is free to use as required and for any purpose
// remember using an el cheapo cpu with hardly any stack so do not nest procs and no fancy programming!

#include <xc.h>

#pragma config WDT = OFF, OSC = IntRC_RB4EN, MCLRE = OFF, CP = OFF // Int 4Mhz Clock, disable MCLR, watchdog timer and Code protection.

const unsigned char Seg7b[] = {0x01,0x07,0x02,0x02,0x04,0x00,0x00,0x03,0x00,0x00}; // 7 segment lookup table for port b rb0=d, rb1=b, rb2=e
const unsigned char Seg7c[] = {0x00,0x06,0x01,0x04,0x06,0x24,0x20,0x06,0x00,0x04}; // 7 segment lookup table for port c

unsigned char ticks,count,seconds,selenoid_state;

unsigned char SelenoidOn() // sample selenoid, wait 5mS then check still same to allow for small glitches (delay must be 5mS as we also use func for timing!)
{
	selenoid_state = RB5; // sample
  TMR0=1;
	while(TMR0); // delay here for 5mS (tweek so about right to strobe LED digits without user noticing flicker)
  if(ticks++ > 188) ticks=0; // tweek this so we reset the 5mS ticks counter to 0 every second (or as exact as we can get)
	return selenoid_state && RB5; // check sample against actual and return selenoid state.
}

void display()
{
	// display off (prevent display ghosting as we multiplex) then alternately write each digit on each 5ms tick
	PORTB=0x07;
	PORTC=0x3f;
	if(seconds<35 || ticks>64) // logic here flashes digits if over 35 seconds
	{
		if(ticks%2) // display digit 1 (units) on even tick
		{
			PORTC = Seg7c[seconds%10] | 0x08; // or digit 1 anode
			PORTB = Seg7b[seconds%10];
		}
		else // display digit 2 (tens) on odd tick
		{
			PORTC = Seg7c[seconds/10] | 0x10; // or digit 2 anode
			PORTB = Seg7b[seconds/10];
		}
	}
}

int main()
{
	// Init PIC16F505 (See datasheet)
  TRISB = 0x20;  // PortB: [All Output] except RB5 (Selenoid trigger input from rectifier)
	TRISC = 0x00;  // PortC: [All output]
	OPTION = 0xC3; // Prescaler to TMR0 (4mhz/4/16 = TMR0 count time). Modify TMR0 preset / C1 rollover values for strobe and second counter

	while(1) // DO Forever
	{
		ticks=0;

		seconds=0;
		while(SelenoidOn()) // while selenoid is ON count and display seconds
		{ 			
			if(seconds>0) display();
  		if(ticks==0) seconds++;
		}

		count=0;
		while(count<20 && seconds>3 && (!SelenoidOn() || count<5)) // hold the display for 5-20 seconds (exit if the selonoid goes active after 5 seconds !)
		{
			display();
			if(ticks==0) count++;
		}

		// display off then wait here until selenoid active so we can repeat all over again
		PORTB=0x07;
		PORTC=0x3f;
		while(!SelenoidOn());
	}
}

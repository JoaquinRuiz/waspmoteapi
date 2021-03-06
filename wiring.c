/*
 *  Modified for Waspmote by D. Cuartielles & A. Bielsa, 2009
 *
 *  Copyright (c) 2008 D. Cuartielles
 *  Copyright (c) 2005-2006 David A. Mellis
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 2.1 of the License, or
 *  (at your option) any later version.
   
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
  
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 

#include "wiring_private.h"

#ifndef __WASPCONSTANTS_H__
#include "WaspConstants.h"
#endif

// general variable declaration

// registers containing the information about
// how to power the different peripherals
uint8_t IPRA = 0;
uint8_t IPRB = 0;


// Wake Up function (empty) to be overridden by the power
// control libraries, and whatever interrupt functions
// the users create
void wakeUpNowDefault()        // here the interrupt is handled after wakeup, this overrides the one in the WaspPWR library
{
  // execute code here after wake-up before returning to the loop() function
  // timers and code using timers (serial.print and more...) will not work here.
  // we don't really need to execute any special functions here, since we
  // just want the thing to wake up
}

volatile uint8_t f_wdt  = 1;
//****************************************************************
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(uint8_t ii) {

	cli();
  f_wdt = 0;
  uint8_t bb;
  uint8_t ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCSR |= (1<<WDCE) | (1<<WDE);

  // set new watchdog timeout value
  WDTCSR = bb;
  WDTCSR |= _BV(WDIE);
	sei();


}
//****************************************************************  
// Watchdog Interrupt Service / is executed when  watchdog timed out
ISR(WDT_vect) {
	cli();
  f_wdt=1;  // set global flag
	WDTCSR |= (1<<WDIF);
  	WDTCSR &= ~(1<<WDIE);               // Disable Watchdog Interrupt Mode
	digitalWrite(WTD_INT_PIN_MON,LOW);
	sei();
}


//----------------------------------------------
void off_watchdog(void)
{
   cli();
   //watchdog_reset();
   MCUSR &= ~(1<<WDRF);

   WDTCSR |= (1<<WDCE) | (1<<WDIE) | (1<<WDE);  // Start timed sequence
   WDTCSR =  0x00;  // turn off WDT
   digitalWrite(WTD_INT_PIN_MON,HIGH);
   sei();
}


// The number of times timer 0 has overflowed since the program started.
// Must be volatile or gcc will optimize away some uses of it.
volatile unsigned long timer0_overflow_count;

SIGNAL(SIG_OVERFLOW0)
{
	timer0_overflow_count++;
}

// The number of times timer 1 has overflowed since the program started.
// Must be volatile or gcc will optimize away some uses of it.
volatile unsigned long timer2_overflow_count;

SIGNAL(SIG_OVERFLOW2)
{
	timer2_overflow_count++;
}

unsigned long millis()
{
	// timer 0 increments every 64 cycles, and overflows when it reaches
	// 256.  we would calculate the total number of clock cycles, then
	// divide by the number of clock cycles per millisecond, but this
	// overflows too often.
	//return timer0_overflow_count * 64UL * 256UL / (F_CPU / 1000UL);
	
	// instead find 1/128th the number of clock cycles and divide by
	// 1/128th the number of clock cycles per millisecond
	return timer0_overflow_count * 64UL * 2UL / (F_CPU / 128000UL);
}

unsigned long millisTim2()
{
	// timer 1 increments every 64 cycles, and overflows when it reaches
	// 256.  we would calculate the total number of clock cycles, then
	// divide by the number of clock cycles per millisecond, but this
	// overflows too often.
	//return timer0_overflow_count * 64UL * 256UL / (F_CPU / 1000UL);
	
	// instead find 1/128th the number of clock cycles and divide by
	// 1/128th the number of clock cycles per millisecond
	return timer2_overflow_count;
}

void delay(unsigned long ms)
{
	unsigned long start = millis();
	
	while (millis() - start < ms);
}

/* Delay for the given number of microseconds.  Assumes a 16 MHz clock. 
 * Disables interrupts, which will disrupt the millis() function if used
 * too frequently. */
void delayMicroseconds(unsigned int us)
{
	uint8_t oldSREG;

	// calling avrlib's delay_us() function with low values (e.g. 1 or
	// 2 microseconds) gives delays longer than desired.
	//delay_us(us);

#if F_CPU >= 16000000L
    // for the 16 MHz clock on most Arduino boards

	// for a one-microsecond delay, simply return.  the overhead
	// of the function call yields a delay of approximately 1 1/8 us.
	if (--us == 0)
		return;

	// the following loop takes a quarter of a microsecond (4 cycles)
	// per iteration, so execute it four times for each microsecond of
	// delay requested.
	us <<= 2;

	// account for the time taken in the preceeding commands.
	us -= 2;
#else
    // for the 8 MHz internal clock on the ATmega168

    // for a one- or two-microsecond delay, simply return.  the overhead of
    // the function calls takes more than two microseconds.  can't just
    // subtract two, since us is unsigned; we'd overflow.
	if (--us == 0)
		return;
	if (--us == 0)
		return;

	// the following loop takes half of a microsecond (4 cycles)
	// per iteration, so execute it twice for each microsecond of
	// delay requested.
	us <<= 1;
    
    // partially compensate for the time taken by the preceeding commands.
    // we can't subtract any more than this or we'd overflow w/ small delays.
    us--;
#endif

	// disable interrupts, otherwise the timer 0 overflow interrupt that
	// tracks milliseconds will make us delay longer than we want.
	oldSREG = SREG;
	cli();

	// busy wait
	__asm__ __volatile__ (
		"1: sbiw %0,1" "\n\t" // 2 cycles
		"brne 1b" : "=w" (us) : "0" (us) // 2 cycles
	);

	// reenable interrupts.
	SREG = oldSREG;
}

/* Wait, or highest sleep level possible keeping the
 * timer1 running, since it is the one counting
 * milliseconds even if the system is asleep */
/*void wait(unsigned long ms)
{
	unsigned long timer2_internal_timeout = ms;
    //timer2_overflow_count = 0;	

    while(timer2_internal_timeout > timer2_overflow_count) {
    	/* Now is the time to set the sleep mode. In the Atmega8 datasheet
    	 * http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf on page 35
    	 * there is a list of sleep modes which explains which clocks and 
    	 * wake up sources are available in which sleep modus.
    	 *
    	 * In the avr/sleep.h file, the call names of these sleep modus are to be found:
    	 *
    	 * The 5 different modes are:
    	 *     SLEEP_MODE_IDLE         -the least power savings 
    	 *     SLEEP_MODE_ADC
    	 *     SLEEP_MODE_PWR_SAVE
    	 *     SLEEP_MODE_STANDBY
    	 *     SLEEP_MODE_PWR_DOWN     -the most power savings
    	 *
    	 * For now, we want as much power savings as possible, so we 
    	 * choose the according 
    	 * sleep modus: SLEEP_MODE_PWR_DOWN
    	 * 
    	 */  
/*    	set_sleep_mode(SLEEP_MODE_PWR_SAVE);   // sleep mode is set here
	
    	sleep_enable();          // enables the sleep bit in the mcucr register
    	                         // so sleep is possible. just a safety pin 
	
    	sleep_mode();            // here the device is actually put to sleep!!
    	                         // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP

    	sleep_disable();         // first thing after waking from sleep:
    	                         // disable sleep...
	}
}*/

// use the internal watchdog to put the system to sleep at maximum value
void wait (uint8_t mode) {
		waitFor( mode, 9);			 // by default we take the longest time available (8secs)
}

// use the internal watchdog to put the system 
// to sleep a certain time under a certain mode
//****************************************************************
// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void waitFor (uint8_t mode, uint8_t time) {
	    cbi(ADCSRA,ADEN);        // switch Analog to Digitalconverter OFF
    	set_sleep_mode(mode);    // sleep mode is set here
		setup_watchdog(time);
    	sleep_enable();          // enables the sleep bit in the mcucr register
    	                         // so sleep is possible. just a safety pin 
	
    	sleep_mode();            // here the device is actually put to sleep!!
    	                         // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP
		off_watchdog();
    	sleep_disable();         // first thing after waking from sleep:
	    sbi(ADCSRA,ADEN);        // switch Analog to Digitalconverter ON
}

void init()
{
	// this needs to be called before setup() or some functions won't
	// work there
	sei();
	
    /* Setup Watchdog */      
    // Use Timed Sequence for disabling Watchdog System Reset Mode if it has been enabled unintentionally.
    cbi(MCUSR, WDRF);                                 // Clear WDRF if it has been unintentionally set.

	// timer 0 is used for millis() and delay()
	timer0_overflow_count = 0;
	// on the ATmega168, timer 0 is also used for fast hardware pwm
	// (using phase-correct PWM would mean that timer 0 overflowed half as often
	// resulting in different millis() behavior on the ATmega8 and ATmega168)
	sbi(TCCR0A, WGM01);
	sbi(TCCR0A, WGM00);

	// set timer 0 prescale factor to 64
	sbi(TCCR0B, CS01);
	sbi(TCCR0B, CS00);

	// enable timer 0 overflow interrupt
	sbi(TIMSK0, TOIE0);

	// timers 1 and 2 are used for phase-correct hardware pwm
	// this is better for motors as it ensures an even waveform
	// note, however, that fast pwm mode can achieve a frequency of up
	// 8 MHz (with a 16 MHz clock) at 50% duty cycle

	// set timer 1 prescale factor to 64
	sbi(TCCR1B, CS11);
	sbi(TCCR1B, CS10);
	// put timer 1 in 8-bit phase correct pwm mode
	sbi(TCCR1A, WGM10);

	// set timer 2 prescale factor to 64
	sbi(TCCR2B, CS22);
	sbi(TCCR3B, CS32);

	// configure timer 2 for phase correct pwm (8-bit)
	sbi(TCCR2A, WGM20);
	sbi(TCCR3A, WGM30);

	// configure and enable a2d conversions
    //FIXME: this is to turn on ADC!! 
	//PWR.setIPF(IPADC); //---> the following is taken from WaspPWR
        // turn on the power on the ADC
        // by writing a zero to the register
        cbi(PRR0, PRADC);

	    // set a2d reference to AVCC (5 volts)
	    cbi(ADMUX, REFS1);
	    sbi(ADMUX, REFS0);

		// set a2d prescale factor to 128
		// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
		// FIXME: this will not work properly for other clock speeds, and
		// this code should use F_CPU to determine the prescale factor.
		sbi(ADCSRA, ADPS2);
		sbi(ADCSRA, ADPS1);

        // enable a2d conversions
        sbi(ADCSRA, ADEN);
	

	// the bootloader connects pins 0 and 1 to the USART; disconnect them
	// here so they can be used as normal digital i/o; they will be
	// reconnected in Serial.begin()
	UCSR0B = 0;
	UCSR1B = 0;
}

//---------------------------------------------------------------------
// general power functions

/* setIPF_ ( peripheral )
 * - sets a certain internal peripheral on 
 * - to control the pwr on the different internal peripherals it is
 *   convenient to read MCU's manual on pgs. 56/57
 * FIXME: missing all the Timers 
 */
void setIPF_(uint8_t peripheral) 
{
    // mark it on the IPFA
    IPRA |= peripheral; 

    // check which flags have been activated
    // ADC, flag IPADC
    if (peripheral & IPADC > 0) {
        // turn on the power on the ADC
        // by writing a zero to the register
        cbi(PRR0, PRADC);

	// set a2d reference to AVCC (5 volts)
	cbi(ADMUX, REFS1);
	sbi(ADMUX, REFS0);

	// set a2d prescale factor to 128
	// 16 MHz / 128 = 125 KHz, inside the desired 50-200 KHz range.
	// FIXME: this will not work properly for other clock speeds, and
	// this code should use F_CPU to determine the prescale factor.
	sbi(ADCSRA, ADPS2);
	sbi(ADCSRA, ADPS1);

        // enable a2d conversions
        sbi(ADCSRA, ADEN);
    }
    // TWI, flag IPTWI
    if (peripheral & IPTWI > 0) {
        // turn on the power on the TWI
        // by writing a zero to the register
        cbi(PRR0, PRTWI);

	// initialize the TWI
        //FIXME: without this reinitialization the peripheral may not work!!
	//Wire.begin();
    }
    // SPI, flag IPSPI (aka SD card)
    if (peripheral & IPSPI > 0) {
        // turn on the power on the SPI
        // by writing a zero to the register
        cbi(PRR0, PRSPI);

	// initialize the SD
        // FIXME: this command is not ready yet, since the library is not finished
	//SD.begin();
    }
    // USART0, flag IPUSART0
    if (peripheral & IPUSART0 > 0) {
        // turn on the power on the USART0
        // by writing a zero to the register
        cbi(PRR0, PRUSART0);
    }
    // USART1, flag IPUSART1
    if (peripheral & IPUSART1 > 0) {
        // turn on the power on the USART0
        // by writing a zero to the register
        cbi(PRR1, PRUSART1);
    }
}

/* resetIPF_ ( peripheral )
 * - resets a certain internal peripheral to off 
 * - to control the pwr on the different internal peripherals it is
 *   convenient to read MCU's manual on pgs. 56/57
 * FIXME: missing all the Timers
 */
void resetIPF_(uint8_t peripheral) 
{
    // mark it on the IPFA
    IPRA &= ~peripheral; 

    // check which flags have been de-activated
    // ADC, flag IPADC
    if (peripheral & IPADC > 0) {
        // disable a2d conversions
        cbi(ADCSRA, ADEN);
        // turn off the power on the ADC
        // by writing a one to the register
        sbi(PRR0, PRADC);
    }
    // TWI, flag IPTWI (also known as I2C)
    if (peripheral & IPTWI > 0) {
        // turn off the power on the TWI
        // by writing a one to the register
        sbi(PRR0, PRTWI);
    }
    // SPI, flag IPSPI (where the SD card hangs)
    if (peripheral & IPSPI > 0) {
        // turn off the power on the SPI
        // by writing a one to the register
        sbi(PRR0, PRSPI);
    }
    // USART0, flag IPUSART0 
    if (peripheral & IPUSART0 > 0) {
        // turn off the power on the USART0
        // by writing a one to the register
        sbi(PRR0, PRUSART0);
    }
    // USART1, flag IPUSART1 
    if (peripheral & IPUSART1 > 0) {
        // turn off the power on the USART0
        // by writing a one to the register
        sbi(PRR1, PRUSART1);
    }
}



/* -----------------------------------------------------------------------------
 * Project:			GPDSE AVR8 Library
 * File:			ATmega328.c
 * Module:			ATmega328 basic interface
 * Author:			Leandro Schwarz
 * Version:			12.2
 * Last edition:	2016-01-15
 * -------------------------------------------------------------------------- */

// -----------------------------------------------------------------------------
// Header files ----------------------------------------------------------------

#include "ATmega328.h"
#if __ATMEGA328_H != 122
	#error Error 101 - Version mismatch on header and source code files (ATmega328).
#endif

// -----------------------------------------------------------------------------
// Global variables ------------------------------------------------------------

FILE usartStream = FDEV_SETUP_STREAM(usartTransmitStd, usartReceiveStd, _FDEV_SETUP_RW);
uint8 usartReceiverBuffer[USART_RECEIVER_BUFFER_SIZE];
uint8 usartReceiverBufferNextRead;	// Pointer to the next read location
uint8 usartReceiverBufferNextWrite;	// Pointer to the next write location
uint8 usartReceiverBufferLength;	// Buffer length

/* -----------------------------------------------------------------------------
 * Enable global interrupts
 * -------------------------------------------------------------------------- */

void globalInterruptEnable(void)
{
	sei();
	return;
}

/* -----------------------------------------------------------------------------
 * Disable global interrupts
 * -------------------------------------------------------------------------- */

void globalInterruptDisable(void)
{
	cli();
	return;
}

/* -----------------------------------------------------------------------------
 * Configures the system clock prescaler
 * -------------------------------------------------------------------------- */

resultValue_t systemClockSetPrescaler(systemPrescalerValue_t prescaler)
{
	uint8 aux8 = 0;

	switch(prescaler){
		case SYSTEM_PRESCALER_OFF:		aux8 = 0;	break;
		case SYSTEM_PRESCALER_2:		aux8 = 1;	break;
		case SYSTEM_PRESCALER_4:		aux8 = 2;	break;
		case SYSTEM_PRESCALER_8:		aux8 = 3;	break;
		case SYSTEM_PRESCALER_16:		aux8 = 4;	break;
		case SYSTEM_PRESCALER_32:		aux8 = 5;	break;
		case SYSTEM_PRESCALER_64:		aux8 = 6;	break;
		case SYSTEM_PRESCALER_128:		aux8 = 7;	break;
		case SYSTEM_PRESCALER_256:		aux8 = 8;	break;
		default:						return RESULT_UNSUPPORTED_VALUE;	break;
	}
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		CLKPR = 0b10000000;
		CLKPR = aux8;
	}

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Changes the EEPROM operation mode
 * -------------------------------------------------------------------------- */

void eepromSetOperationMode(eepromMode_t mode)
{
	uint8 aux8 = 0;

	aux8 = EECR & ~(0x03 << EEPM0);
	aux8 |= (mode << EEPM0);
	EECR = aux8;

	return;
}

/* -----------------------------------------------------------------------------
 * Activates the EEPROM ready interrupt
 * -------------------------------------------------------------------------- */

void eepromReadyActivateInterrupt(void)
{
	setBit(EECR, EERIE);
	return;
}

/* -----------------------------------------------------------------------------
 * Deactivates the EEPROM ready interrupt
 * -------------------------------------------------------------------------- */

void eepromReadyDeactivateInterrupt(void)
{
	clrBit(EECR, EERIE);
	return;
}

/* -----------------------------------------------------------------------------
 * Writes a byte in the specified address in the EEPROM
 * -------------------------------------------------------------------------- */

void eepromWrite(uint16 address, uint8 data)
{
	waitUntilBitIsClear(EECR, EEPE);
	waitUntilBitIsClear(SPMEN, SPMCSR);
	EEAR = (address & EEPROM_ADDRESS_MASK);
	EEDR = data;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		setBit(EECR, EEMPE);
		setBit(EECR, EEPE);
	}

	return;
}

/* -----------------------------------------------------------------------------
 * Reads a byte from the specified address in the EEPROM
 * -------------------------------------------------------------------------- */

uint8 eepromRead(uint16 address)
{
	waitUntilBitIsClear(EECR, EEPE);
	EEAR = (address & EEPROM_ADDRESS_MASK);
	setBit(EECR, EERE);
	return EEDR;
}

/* -----------------------------------------------------------------------------
 * Enables the global pull-up
 * -------------------------------------------------------------------------- */

void globalPullUpEnable(void)
{
	clrBit(MCUCR, PUD);
	return;
}

/* -----------------------------------------------------------------------------
 * Disables the global pull-up
 * -------------------------------------------------------------------------- */

void pullUpDisable(void)
{
	setBit(MCUCR, PUD);
	return;
}

/* -----------------------------------------------------------------------------
 * Configures the external interrupt int0
 * -------------------------------------------------------------------------- */

resultValue_t int0Config(portMode_t port, senseMode_t sense)
{
	uint8 aux8 = 0;

	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD2); setBit(DDRD, PD2);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD2); setBit(DDRD, PD2);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD2); clrBit(DDRD, PD2);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD2); clrBit(DDRD, PD2);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}

	if(sense != SENSE_NO_CHANGE){
		aux8 = EICRA & ~(0x03 << ISC00);
		switch(sense){
			case SENSE_LOW_LEVEL:		break;
			case SENSE_ANY_EDGE:		aux8 |= (0x01 << ISC00);			break;
			case SENSE_FALLING_EDGE:	aux8 |= (0x02 << ISC00);			break;
			case SENSE_RISING_EDGE:		aux8 |= (0x03 << ISC00);			break;
			case SENSE_NO_CHANGE:		break;
			default:					return RESULT_INT_SENSE_VALUE_UNSUPPORTED;	break;
		}
		EICRA = aux8;
	}

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt int0
 * -------------------------------------------------------------------------- */

void int0ActivateInterrupt(void)
{
	setBit(EIMSK, INT0);
	return;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt int0
 * -------------------------------------------------------------------------- */

void int0DeactivateInterrupt(void)
{
	clrBit(EIMSK, INT0);
	return;
}

/* -----------------------------------------------------------------------------
 * Clears the external interrupt int0 request
 * -------------------------------------------------------------------------- */

void int0ClearInterruptRequest(void)
{
	setBit(EIFR, INTF0);
	return;
}

/* -----------------------------------------------------------------------------
 * Configures the external interrupt int1
 * -------------------------------------------------------------------------- */

resultValue_t int1Config(portMode_t port, senseMode_t sense)
{
	uint8 aux8 = 0;

	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD3); setBit(DDRD, PD3);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD3); setBit(DDRD, PD3);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD3); clrBit(DDRD, PD3);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD3); clrBit(DDRD, PD3);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}

	if(sense != SENSE_NO_CHANGE){
		aux8 = EICRA & ~(0x03 << ISC10);
		switch(sense){
			case SENSE_LOW_LEVEL:		break;
			case SENSE_ANY_EDGE:		aux8 |= (0x01 << ISC10);			break;
			case SENSE_FALLING_EDGE:	aux8 |= (0x02 << ISC10);			break;
			case SENSE_RISING_EDGE:		aux8 |= (0x03 << ISC10);			break;
			case SENSE_NO_CHANGE:		break;
			default:					return RESULT_INT_SENSE_VALUE_UNSUPPORTED;	break;
		}
		EICRA = aux8;
	}

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt int1
 * -------------------------------------------------------------------------- */

void int1ActivateInterrupt(void)
{
	setBit(EIMSK, INT1);
	return;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt int1
 * -------------------------------------------------------------------------- */

void int1DeactivateInterrupt(void)
{
	clrBit(EIMSK, INT1);
	return;
}

/* -----------------------------------------------------------------------------
 * Clears the external interrupt int1 request
 * -------------------------------------------------------------------------- */

void int1ClearInterruptRequest(void)
{
	setBit(EIFR, INTF1);
	return;
}

/* -----------------------------------------------------------------------------
 * Enables the external interrupt pcint in port B
 * -------------------------------------------------------------------------- */

void pcint7_0Enable(void)
{
	setBit(PCICR, PCIE0);
	return;
}

/* -----------------------------------------------------------------------------
 * Disables the external interrupt pcint in port B
 * -------------------------------------------------------------------------- */

void pcint7_0Disable(void)
{
	clrBit(PCICR, PCIE0);
	return;
}

/* -----------------------------------------------------------------------------
 * Disables the external interrupt pcint in port B
 * -------------------------------------------------------------------------- */

void pcint7_0ClearInterruptRequest(void)
{
	setBit(PCIFR, PCIF0);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint0 and configures port mode in PB0
 * -------------------------------------------------------------------------- */

resultValue_t pcint0ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB0); setBit(DDRB, PB0);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB0); setBit(DDRB, PB0);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB0); clrBit(DDRB, PB0);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB0); clrBit(DDRB, PB0);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint0 in PB0
 * -------------------------------------------------------------------------- */

void pcint0DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT0);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint1 and configures port mode in PB1
 * -------------------------------------------------------------------------- */

resultValue_t pcint1ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB1); setBit(DDRB, PB1);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB1); setBit(DDRB, PB1);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB1); clrBit(DDRB, PB1);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB1); clrBit(DDRB, PB1);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT1);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint1 in PB1
 * -------------------------------------------------------------------------- */

void pcint1DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT1);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint2 and configures port mode in PB2
 * -------------------------------------------------------------------------- */

resultValue_t pcint2ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB2); setBit(DDRB, PB2);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB2); setBit(DDRB, PB2);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB2); clrBit(DDRB, PB2);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB2); clrBit(DDRB, PB2);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT2);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint2 in PB2
 * -------------------------------------------------------------------------- */

void pcint2DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT2);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint3 and configures port mode in PB3
 * -------------------------------------------------------------------------- */

resultValue_t pcint3ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB3); setBit(DDRB, PB3);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB3); setBit(DDRB, PB3);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB3); clrBit(DDRB, PB3);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB3); clrBit(DDRB, PB3);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT3);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint3 in PB3
 * -------------------------------------------------------------------------- */

void pcint3DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT3);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint4 and configures port mode in PB4
 * -------------------------------------------------------------------------- */

resultValue_t pcint4ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB4); setBit(DDRB, PB4);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB4); setBit(DDRB, PB4);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB4); clrBit(DDRB, PB4);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB4); clrBit(DDRB, PB4);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT4);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint4 in PB4
 * -------------------------------------------------------------------------- */

void pcint4DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT4);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint5 and configures port mode in PB5
 * -------------------------------------------------------------------------- */

resultValue_t pcint5ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB5); setBit(DDRB, PB5);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB5); setBit(DDRB, PB5);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB5); clrBit(DDRB, PB5);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB5); clrBit(DDRB, PB5);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT5);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint5 in PB5
 * -------------------------------------------------------------------------- */

void pcint5DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT5);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint6 and configures port mode in PB6
 * -------------------------------------------------------------------------- */

resultValue_t pcint6ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB6); setBit(DDRB, PB6);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB6); setBit(DDRB, PB6);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB6); clrBit(DDRB, PB6);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB6); clrBit(DDRB, PB6);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT6);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint6 in PB6
 * -------------------------------------------------------------------------- */

void pcint6DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT6);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint7 and configures port mode in PB7
 * -------------------------------------------------------------------------- */

resultValue_t pcint7ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTB, PB7); setBit(DDRB, PB7);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTB, PB7); setBit(DDRB, PB7);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTB, PB7); clrBit(DDRB, PB7);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTB, PB7); clrBit(DDRB, PB7);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK0, PCINT7);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint7 in PB7
 * -------------------------------------------------------------------------- */

void pcint7DeactivateInterrupt(void)
{
	clrBit(PCMSK0, PCINT7);
	return;
}

/* -----------------------------------------------------------------------------
 * Enables the external interrupt pcint in port C
 * -------------------------------------------------------------------------- */

void pcint14_8Enable(void)
{
	setBit(PCICR, PCIE1);
	return;
}

/* -----------------------------------------------------------------------------
 * Disables the external interrupt pcint in port C
 * -------------------------------------------------------------------------- */

void pcint14_8Disable(void)
{
	clrBit(PCICR, PCIE1);
	return;
}

/* -----------------------------------------------------------------------------
 * Disables the external interrupt pcint in port C
 * -------------------------------------------------------------------------- */

void pcint14_8ClearInterruptRequest(void)
{
	setBit(PCIFR, PCIF1);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint8 and configures port mode in PC0
 * -------------------------------------------------------------------------- */

resultValue_t pcint8ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTC, PC0); setBit(DDRC, PC0);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTC, PC0); setBit(DDRC, PC0);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTC, PC0); clrBit(DDRC, PC0);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTC, PC0); clrBit(DDRC, PC0);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK1, PCINT8);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint8 in PC0
 * -------------------------------------------------------------------------- */

void pcint8DeactivateInterrupt(void)
{
	clrBit(PCMSK1, PCINT8);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint9 and configures port mode in PC1
 * -------------------------------------------------------------------------- */

resultValue_t pcint9ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTC, PC1); setBit(DDRC, PC1);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTC, PC1); setBit(DDRC, PC1);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTC, PC1); clrBit(DDRC, PC1);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTC, PC1); clrBit(DDRC, PC1);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK1, PCINT9);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint9 in PC1
 * -------------------------------------------------------------------------- */

void pcint9DeactivateInterrupt(void)
{
	clrBit(PCMSK1, PCINT9);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint10 and configures port mode in PC2
 * -------------------------------------------------------------------------- */

resultValue_t pcint10ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTC, PC2); setBit(DDRC, PC2);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTC, PC2); setBit(DDRC, PC2);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTC, PC2); clrBit(DDRC, PC2);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTC, PC2); clrBit(DDRC, PC2);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK1, PCINT10);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint10 in PC2
 * -------------------------------------------------------------------------- */

void pcint10DeactivateInterrupt(void)
{
	clrBit(PCMSK1, PCINT10);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint11 and configures port mode in PC3
 * -------------------------------------------------------------------------- */

resultValue_t pcint11ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTC, PC3); setBit(DDRC, PC3);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTC, PC3); setBit(DDRC, PC3);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTC, PC3); clrBit(DDRC, PC3);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTC, PC3); clrBit(DDRC, PC3);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK1, PCINT11);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint11 in PC3
 * -------------------------------------------------------------------------- */

void pcint11DeactivateInterrupt(void)
{
	clrBit(PCMSK1, PCINT11);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint12 and configures port mode in PC4
 * -------------------------------------------------------------------------- */

resultValue_t pcint12ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTC, PC4); setBit(DDRC, PC4);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTC, PC4); setBit(DDRC, PC4);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTC, PC4); clrBit(DDRC, PC4);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTC, PC4); clrBit(DDRC, PC4);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK1, PCINT12);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint12 in PC4
 * -------------------------------------------------------------------------- */

void pcint12DeactivateInterrupt(void)
{
	clrBit(PCMSK1, PCINT12);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint13 and configures port mode in PC5
 * -------------------------------------------------------------------------- */

resultValue_t pcint13ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTC, PC5); setBit(DDRC, PC5);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTC, PC5); setBit(DDRC, PC5);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTC, PC5); clrBit(DDRC, PC5);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTC, PC5); clrBit(DDRC, PC5);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK1, PCINT13);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint13 in PC5
 * -------------------------------------------------------------------------- */

void pcint13DeactivateInterrupt(void)
{
	clrBit(PCMSK1, PCINT13);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint14 and configures port mode in PC6
 * -------------------------------------------------------------------------- */

resultValue_t pcint14ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTC, PC6); setBit(DDRC, PC6);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTC, PC6); setBit(DDRC, PC6);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTC, PC6); clrBit(DDRC, PC6);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTC, PC6); clrBit(DDRC, PC6);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK1, PCINT14);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint14 in PC6
 * -------------------------------------------------------------------------- */

void pcint14DeactivateInterrupt(void)
{
	clrBit(PCMSK1, PCINT14);
	return;
}

/* -----------------------------------------------------------------------------
 * Enables the external interrupt pcint in port D
 * -------------------------------------------------------------------------- */

void pcint23_16Enable(void)
{
	setBit(PCICR, PCIE2);
	return;
}

/* -----------------------------------------------------------------------------
 * Disables the external interrupt pcint in port D
 * -------------------------------------------------------------------------- */

void pcint23_16Disable(void)
{
	clrBit(PCICR, PCIE2);
	return;
}

/* -----------------------------------------------------------------------------
 * Disables the external interrupt pcint in port D
 * -------------------------------------------------------------------------- */

void pcint23_16ClearInterruptRequest(void)
{
	setBit(PCIFR, PCIF2);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint16 and configures port mode in PD0
 * -------------------------------------------------------------------------- */

resultValue_t pcint16ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD0); setBit(DDRD, PD0);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD0); setBit(DDRD, PD0);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD0); clrBit(DDRD, PD0);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD0); clrBit(DDRD, PD0);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT16);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint16 in PD0
 * -------------------------------------------------------------------------- */

void pcint16DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT16);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint17 and configures port mode in PD1
 * -------------------------------------------------------------------------- */

resultValue_t pcint17ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD1); setBit(DDRD, PD1);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD1); setBit(DDRD, PD1);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD1); clrBit(DDRD, PD1);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD1); clrBit(DDRD, PD1);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT17);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint17 in PD1
 * -------------------------------------------------------------------------- */

void pcint17DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT17);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint18 and configures port mode in PD2
 * -------------------------------------------------------------------------- */

resultValue_t pcint18ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD2); setBit(DDRD, PD2);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD2); setBit(DDRD, PD2);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD2); clrBit(DDRD, PD2);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD2); clrBit(DDRD, PD2);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT18);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint18 in PD2
 * -------------------------------------------------------------------------- */

void pcint18DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT18);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint19 and configures port mode in PD3
 * -------------------------------------------------------------------------- */

resultValue_t pcint19ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD3); setBit(DDRD, PD3);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD3); setBit(DDRD, PD3);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD3); clrBit(DDRD, PD3);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD3); clrBit(DDRD, PD3);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT19);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint19 in PD3
 * -------------------------------------------------------------------------- */

void pcint19DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT19);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint20 and configures port mode in PD4
 * -------------------------------------------------------------------------- */

resultValue_t pcint20ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD4); setBit(DDRD, PD4);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD4); setBit(DDRD, PD4);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD4); clrBit(DDRD, PD4);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD4); clrBit(DDRD, PD4);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT20);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint20 in PD4
 * -------------------------------------------------------------------------- */

void pcint20DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT20);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint21 and configures port mode in PD5
 * -------------------------------------------------------------------------- */

resultValue_t pcint21ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD5); setBit(DDRD, PD5);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD5); setBit(DDRD, PD5);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD5); clrBit(DDRD, PD5);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD5); clrBit(DDRD, PD5);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT21);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint21 in PD5
 * -------------------------------------------------------------------------- */

void pcint21DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT21);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint22 and configures port mode in PD6
 * -------------------------------------------------------------------------- */

resultValue_t pcint22ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD6); setBit(DDRD, PD6);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD6); setBit(DDRD, PD6);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD6); clrBit(DDRD, PD6);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD6); clrBit(DDRD, PD6);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT22);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint22 in PD6
 * -------------------------------------------------------------------------- */

void pcint22DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT22);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the external interrupt pcint23 and configures port mode in PD7
 * -------------------------------------------------------------------------- */

resultValue_t pcint23ActivateInterrupt(portMode_t port)
{
	switch(port){
		case PORT_OUTPUT_LOW:		clrBit(PORTD, PD7); setBit(DDRD, PD7);	break;
		case PORT_OUTPUT_HIGH:		setBit(PORTD, PD7); setBit(DDRD, PD7);	break;
		case PORT_INPUT_TRISTATE:	clrBit(PORTD, PD7); clrBit(DDRD, PD7);	break;
		case PORT_INPUT_PULL_UP:	setBit(PORTD, PD7); clrBit(DDRD, PD7);	break;
		case PORT_NO_CHANGE:		break;
		default:					return RESULT_PORT_VALUE_UNSUPPORTED;	break;
	}
	setBit(PCMSK2, PCINT23);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the external interrupt pcint23 in PD7
 * -------------------------------------------------------------------------- */

void pcint23DeactivateInterrupt(void)
{
	clrBit(PCMSK2, PCINT23);
	return;
}

/* -----------------------------------------------------------------------------
 * Configures the timer0 mode and prescaler
 * -------------------------------------------------------------------------- */

resultValue_t timer0Config(timerModeA_t mode, timerPrescalerValueA_t prescaler)
{
	uint8 regA = TCCR0A;
	uint8 regB = TCCR0B;

	if(prescaler != TIMER_A_PRESCALER_NO_CHANGE){
		regB &= ~(0x07 << CS00);
		switch(prescaler){
			case TIMER_A_CLOCK_DISABLE:					break;
			case TIMER_A_PRESCALER_OFF:					regB |= (1 << CS00);	break;
			case TIMER_A_PRESCALER_8:					regB |= (2 << CS00);	break;
			case TIMER_A_PRESCALER_64:					regB |= (3 << CS00);	break;
			case TIMER_A_PRESCALER_256:					regB |= (4 << CS00);	break;
			case TIMER_A_PRESCALER_1024:				regB |= (5 << CS00);	break;
			case TIMER_A_PRESCALER_T0_FALLING_EDGE:		regB |= (6 << CS00);	break;
			case TIMER_A_PRESCALER_T0_RISING_EDGE:		regB |= (7 << CS00);	break;
			case TIMER_A_PRESCALER_NO_CHANGE:			break;
			default:									return RESULT_TIMER_PRESCALER_VALUE_UNSUPPORTED;	break;
		}
	}

	if(mode != TIMER_A_MODE_NO_CHANGE){
		regA &= ~(0x03 << WGM00);
		clrBit(regB, WGM02);
		switch(mode){
			case TIMER_A_MODE_NORMAL:						break;
			case TIMER_A_MODE_PWM_PHASE_CORRECTED_MAX:		regA |= (1 << WGM00);	break;
			case TIMER_A_MODE_CTC:							regA |= (2 << WGM00);	break;
			case TIMER_A_MODE_FAST_PWM_MAX:					regA |= (3 << WGM00);	break;
			case TIMER_A_MODE_PWM_PHASE_CORRECTED_OCRA:		setBit(regB, WGM02);regA |= (1 << WGM00);	break;
			case TIMER_A_MODE_FAST_PWM_OCRA:				setBit(regB, WGM02);regA |= (3 << WGM00);	break;
			case TIMER_A_MODE_NO_CHANGE:					break;
			default:										return RESULT_TIMER_MODE_VALUE_UNSUPPORTED;	break;
		}
	}
	TCCR0A = regA;
	TCCR0B = regB;

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Configures the timer0 compare outputs
 * -------------------------------------------------------------------------- */

resultValue_t timer0OutputConfig(timerOutput_t compA, timerOutput_t compB)
{
	uint8 reg = TCCR0A;

	if(compA != TIMER_PORT_NO_CHANGE){
		reg &= ~(0x03 << COM0A0);
		switch(compA){
			case TIMER_PORT_NORMAL:				break;
			case TIMER_PORT_TOGGLE_ON_COMPARE:	reg |= (1 << COM0A0);	break;
			case TIMER_PORT_CLEAR_ON_COMPARE:	reg |= (2 << COM0A0);	break;
			case TIMER_PORT_SET_ON_COMPARE:		reg |= (3 << COM0A0);	break;
			case TIMER_PORT_NO_CHANGE:			break;
			default:							return RESULT_PORT_VALUE_UNSUPPORTED;	break;
		}
	}
	if(compB != TIMER_PORT_NO_CHANGE){
		reg &= ~(0x03 << COM0B0);
		switch(compA){
			case TIMER_PORT_NORMAL:				break;
			case TIMER_PORT_TOGGLE_ON_COMPARE:	reg |= (1 << COM0B0);	break;
			case TIMER_PORT_CLEAR_ON_COMPARE:	reg |= (2 << COM0B0);	break;
			case TIMER_PORT_SET_ON_COMPARE:		reg |= (3 << COM0B0);	break;
			case TIMER_PORT_NO_CHANGE:			break;
			default:							return RESULT_PORT_VALUE_UNSUPPORTED;	break;
		}
	}
	TCCR0A = reg;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer0 overflow interrupt
 * -------------------------------------------------------------------------- */

void timer0ActivateOverflowInterrupt(void)
{
	setBit(TIMSK0, TOIE0);
	return;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer0 overflow interrupt
 * -------------------------------------------------------------------------- */

void timer0DeactivateOverflowInterrupt(void)
{
	clrBit(TIMSK0, TOIE0);
	return;
}

/* -----------------------------------------------------------------------------
 * Clears the timer0 overflow interrupt request
 * -------------------------------------------------------------------------- */

void timer0ClearOverflowInterruptRequest(void)
{
	setBit(TIFR0, TOV0);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the timer0 compare A interrupt
 * -------------------------------------------------------------------------- */

void timer0ActivateCompareAInterrupt(void)
{
	setBit(TIMSK0, OCIE0A);
	return;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer0 compare A interrupt
 * -------------------------------------------------------------------------- */

void timer0DeactivateCompareAInterrupt(void)
{
	clrBit(TIMSK0, OCIE0A);
	return;
}

/* -----------------------------------------------------------------------------
 * Clears the timer0 compare A interrupt request
 * -------------------------------------------------------------------------- */

void timer0ClearCompareAInterruptRequest(void)
{
	setBit(TIFR0, OCF0A);
	return;
}

/* -----------------------------------------------------------------------------
 * Activates the timer0 compare B interrupt
 * -------------------------------------------------------------------------- */

void timer0ActivateCompareBInterrupt(void)
{
	setBit(TIMSK0, OCIE0B);
	return;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer0 compare B interrupt
 * -------------------------------------------------------------------------- */

void timer0DeactivateCompareBInterrupt(void)
{
	clrBit(TIMSK0, OCIE0B);
	return;
}

/* -----------------------------------------------------------------------------
 * Clears the timer0 compare B interrupt request
 * -------------------------------------------------------------------------- */

void timer0ClearCompareBInterruptRequest(void)
{
	setBit(TIFR0, OCF0B);
	return;
}

/* -----------------------------------------------------------------------------
 * Forces a comparison on the output compare A of the timer0
 * -------------------------------------------------------------------------- */

void timer0ForceCompareA(void)
{
	setBit(TCCR0B, FOC0A);
	return;
}

/* -----------------------------------------------------------------------------
 * Forces a comparison on the output compare B of the timer0
 * -------------------------------------------------------------------------- */

void timer0ForceCompareB(void)
{
	setBit(TCCR0B, FOC0B);
	return;
}

/* -----------------------------------------------------------------------------
 * Sets the timer0 counter value
 * -------------------------------------------------------------------------- */

void timer0SetCounterValue(uint8 value)
{
	TCNT0 = value;
	return;
}

/* -----------------------------------------------------------------------------
 * Gets the timer0 counter value
 * -------------------------------------------------------------------------- */

uint8 timer0GetCounterValue(void)
{
	return TCNT0;
}

/* -----------------------------------------------------------------------------
 * Sets the timer0 compare A value
 * -------------------------------------------------------------------------- */

void timer0SetCompareAValue(uint8 value)
{
	OCR0A = value;
	return;
}

/* -----------------------------------------------------------------------------
 * Gets the timer0 compare A value
 * -------------------------------------------------------------------------- */

uint8 timer0GetCompareAValue(void)
{
	return OCR0A;
}

/* -----------------------------------------------------------------------------
 * Sets the timer0 compare B value
 * -------------------------------------------------------------------------- */

void timer0SetCompareBValue(uint8 value)
{
	OCR0B = value;
	return;
}

/* -----------------------------------------------------------------------------
 * Gets the timer0 compare B value
 * -------------------------------------------------------------------------- */

uint8 timer0GetCompareBValue(void)
{
	return OCR0B;
}









/* -----------------------------------------------------------------------------
 * Configures the timer1 mode and prescaler
 * -------------------------------------------------------------------------- */

resultValue_t timer1Config(timerModeB_t mode, timerPrescalerValueA_t prescaler)
{
	uint8 reg1 = TCCR1A;
	uint8 reg2 = TCCR1B;

	if(prescaler != TIMER_A_PRESCALER_NO_CHANGE){
		reg2 &= ~(0x07 << CS10);
		reg2 |= prescaler;
	}

	if(mode != TIMER_B_MODE_NO_CHANGE){
		if(isBitSet(mode, 3))
			setBit(reg2, WGM13);
		else
			clrBit(reg2, WGM13);
		if(isBitSet(mode, 2))
			setBit(reg2, WGM12);
		else
			clrBit(reg2, WGM12);
		reg1 &= ~(0x03 << WGM10);
		reg1 |= (mode & 0x03);
	}
	TCCR1A = reg1;
	TCCR1B = reg2;

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Configures the timer1 compare outputs
 * -------------------------------------------------------------------------- */

resultValue_t timer1OutputConfig(timerOutput_t compA, timerOutput_t compB)
{
	uint8 aux8 = TCCR1A;

	if(compA != TIMER_PORT_NO_CHANGE){
		aux8 &= ~(0x03 << COM1A0);
		aux8 |= (compA << COM1A0); 
	}
	if(compB != TIMER_PORT_NO_CHANGE){
		aux8 &= ~(0x03 << COM1B0);
		aux8 |= (compB << COM1B0); 
	}
	TCCR1A = aux8;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Configures the noise canceler of input capture of timer1
 * -------------------------------------------------------------------------- */

resultValue_t	timer1InputCaptureNoiseCancelerConfig(timerInputCaptureNoiseCanceler_t mode)
{
	uint8 aux8 = TCCR1B;

	aux8 &= ~(0x03 << ICES1);
	aux8 |= mode;

	TCCR1B = aux8;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer1 overflow interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1ActivateOverflowInterrupt(void)
{
	setBit(TIMSK1, TOIE1);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer1 overflow interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1DeactivateOverflowInterrupt(void)
{
	clrBit(TIMSK1, TOIE1);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the timer1 overflow interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t timer1ClearOverflowInterruptRequest(void)
{
	setBit(TIFR1, TOV1);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer1 compare A interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1ActivateCompareAInterrupt(void)
{
	setBit(TIMSK1, OCIE1A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer1 compare A interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1DeactivateCompareAInterrupt(void)
{
	clrBit(TIMSK1, OCIE1A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the timer1 compare A interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t timer1ClearCompareAInterruptRequest(void)
{
	setBit(TIFR1, OCF1A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer1 compare B interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1ActivateCompareBInterrupt(void)
{
	setBit(TIMSK1, OCIE1B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer1 compare B interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1DeactivateCompareBInterrupt(void)
{
	clrBit(TIMSK1, OCIE1B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the timer1 compare B interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t timer1ClearCompareBInterruptRequest(void)
{
	setBit(TIFR1, OCF1B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer1 input capture interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1ActivateInputCaptureInterrupt(void)
{
	setBit(TIMSK1, ICIE1);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer1 input capture interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer1DeactivateInputCaptureInterrupt(void)
{
	clrBit(TIMSK1, ICIE1);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the timer1 input capture interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t timer1ClearInputCaptureInterruptRequest(void)
{
	setBit(TIFR1, ICF1);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Forces a comparison on the output compare A of the timer1
 * -------------------------------------------------------------------------- */

resultValue_t timer1ForceCompareA(void)
{
	setBit(TCCR1C, FOC1A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Forces a comparison on the output compare B of the timer1
 * -------------------------------------------------------------------------- */

resultValue_t timer1ForceCompareB(void)
{
	setBit(TCCR1C, FOC1B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Sets the timer1 counter value
 * -------------------------------------------------------------------------- */

resultValue_t timer1SetCounterValue(uint16 value)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		TCNT1 = value;
	}
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Gets the timer1 counter value
 * -------------------------------------------------------------------------- */

uint16 timer1GetCounterValue(void)
{
	return TCNT1;
}

/* -----------------------------------------------------------------------------
 * Sets the timer1 compare A value
 * -------------------------------------------------------------------------- */

resultValue_t timer1SetCompareAValue(uint16 value)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		OCR1A = value;
	}
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Gets the timer1 compare A value
 * -------------------------------------------------------------------------- */

uint16 timer1GetCompareAValue(void)
{
	return OCR1A;
}

/* -----------------------------------------------------------------------------
 * Sets the timer1 compare B value
 * -------------------------------------------------------------------------- */

resultValue_t timer1SetCompareBValue(uint16 value)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		OCR1B = value;
	}
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Gets the timer1 compare B value
 * -------------------------------------------------------------------------- */

uint16 timer1GetCompareBValue(void)
{
	return OCR1B;
}

/* -----------------------------------------------------------------------------
 * Sets the timer1 input capture value
 * -------------------------------------------------------------------------- */

resultValue_t timer1SetInputCaptureValue(uint16 value)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
		ICR1 = value;
	}
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Gets the timer1 input capture value
 * -------------------------------------------------------------------------- */

uint16 timer1GetInputCaptureValue(void)
{
	return ICR1;
}

/* -----------------------------------------------------------------------------
 * Configures the timer2 mode and prescaler
 * -------------------------------------------------------------------------- */

resultValue_t timer2Config(timerModeA_t mode, timerPrescalerValueB_t prescaler)
{
	uint8 reg1 = TCCR2A;
	uint8 reg2 = TCCR2B;

	if(prescaler != TIMER_B_PRESCALER_NO_CHANGE){
		reg2 &= ~(0x07 << CS20);
		reg2 |= prescaler;
	}

	if(mode != TIMER_A_MODE_NO_CHANGE){
		if(isBitSet(mode, 2))
			setBit(reg2, WGM22);
		else
			clrBit(reg2, WGM22);
		reg1 &= ~(0x03 << WGM20);
		reg1 |= (mode & 0x03);
	}
	TCCR2A = reg1;
	TCCR2B = reg2;

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Configures the timer2 compare outputs
 * -------------------------------------------------------------------------- */

resultValue_t timer2OutputConfig(timerOutput_t compA, timerOutput_t compB)
{
	uint8 aux8 = TCCR2A;

	if(compA != TIMER_PORT_NO_CHANGE){
		aux8 &= ~(0x03 << COM2A0);
		aux8 |= (compA << COM2A0); 
	}
	if(compB != TIMER_PORT_NO_CHANGE){
		aux8 &= ~(0x03 << COM2B0);
		aux8 |= (compB << COM2B0); 
	}
	TCCR2A = aux8;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer2 overflow interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer2ActivateOverflowInterrupt(void)
{
	setBit(TIMSK2, TOIE2);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer2 overflow interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer2DeactivateOverflowInterrupt(void)
{
	clrBit(TIMSK2, TOIE2);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the timer2 overflow interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t timer2ClearOverflowInterruptRequest(void)
{
	setBit(TIFR2, TOV2);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer2 compare A interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer2ActivateCompareAInterrupt(void)
{
	setBit(TIMSK2, OCIE2A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer2 compare A interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer2DeactivateCompareAInterrupt(void)
{
	clrBit(TIMSK2, OCIE2A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the timer2 compare A interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t timer2ClearCompareAInterruptRequest(void)
{
	setBit(TIFR2, OCF2A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates the timer2 compare B interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer2ActivateCompareBInterrupt(void)
{
	setBit(TIMSK2, OCIE2B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates the timer2 compare B interrupt
 * -------------------------------------------------------------------------- */

resultValue_t timer2DeactivateCompareBInterrupt(void)
{
	clrBit(TIMSK2, OCIE2B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the timer2 compare B interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t timer2ClearCompareBInterruptRequest(void)
{
	setBit(TIFR2, OCF2B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Forces a comparison on the output compare A of the timer2
 * -------------------------------------------------------------------------- */

resultValue_t timer2ForceCompareA(void)
{
	setBit(TCCR2B, FOC2A);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Forces a comparison on the output compare B of the timer2
 * -------------------------------------------------------------------------- */

resultValue_t timer2ForceCompareB(void)
{
	setBit(TCCR2B, FOC2B);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Sets the timer2 counter value
 * -------------------------------------------------------------------------- */

resultValue_t timer2SetCounterValue(uint8 value)
{
	TCNT2 = value;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Gets the timer2 counter value
 * -------------------------------------------------------------------------- */

uint8 timer2GetCounterValue(void)
{
	return TCNT2;
}

/* -----------------------------------------------------------------------------
 * Sets the timer2 compare A value
 * -------------------------------------------------------------------------- */

resultValue_t timer2SetCompareAValue(uint8 value)
{
	OCR2A = value;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Gets the timer2 compare A value
 * -------------------------------------------------------------------------- */

uint8 timer2GetCompareAValue(void)
{
	return OCR2A;
}

/* -----------------------------------------------------------------------------
 * Sets the timer2 compare B value
 * -------------------------------------------------------------------------- */

resultValue_t timer2SetCompareBValue(uint8 value)
{
	OCR2B = value;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Gets the timer2 compare B value
 * -------------------------------------------------------------------------- */

uint8 timer2GetCompareBValue(void)
{
	return OCR2B;
}

/* -----------------------------------------------------------------------------
 * Configures the adc module
 * -------------------------------------------------------------------------- */

resultValue_t adcConfig(adcMode_t mode, adcReference_t reference, adcPrescaler_t  prescaler)
{
	uint8 reg = 0;

	if(reference != ADC_REFERENCE_NO_CHANGE){
		reg = ADMUX;
		reg &= ~(0x03 << REFS0);
		reg |= reference << REFS0;
		ADMUX = reg;
	}

	if(prescaler != ADC_PRESCALER_NO_CHANGE){
		reg = ADCSRA;
		reg &= ~(0x07 << ADPS0);
		reg |= prescaler << ADPS0;
		ADCSRA = reg;
	}

	if(mode != ADC_MODE_NO_CHANGE){
		if(mode == ADC_MODE_SINGLE_CONVERSION)
			clrBit(ADCSRA, ADATE);
		else{
			reg = ADCSRB;
			reg &= ~(0x07 << ADTS0);
			switch(mode){
				case ADC_MODE_AUTO_ANALOG_COMP:		reg |= (1 << ADTS0);	break;
				case ADC_MODE_AUTO_INT0:			reg |= (2 << ADTS0);	break;
				case ADC_MODE_AUTO_TIMER0_COMPA:	reg |= (3 << ADTS0);	break;
				case ADC_MODE_AUTO_TIMER0_OVERFLOW:	reg |= (4 << ADTS0);	break;
				case ADC_MODE_AUTO_TIMER1_COMPB:	reg |= (5 << ADTS0);	break;
				case ADC_MODE_AUTO_TIMER1_OVERFLOW:	reg |= (6 << ADTS0);	break;
				case ADC_MODE_AUTO_TIMER1_CAPTURE:	reg |= (7 << ADTS0);	break;
				default:							return RESULT_UNSUPPORTED_VALUE;	break;
			}
			ADCSRB = reg;
			setBit(ADCSRA, ADATE);
		}
	}

	return RESULT_OK;
 }

/* -----------------------------------------------------------------------------
 * Selects the adc channel
 * -------------------------------------------------------------------------- */

resultValue_t adcSelectChannel(adcChannel_t channel)
{
	uint8 reg = ADMUX;

	reg &= ~(0x0F << MUX0);
	reg |= (channel << MUX0);
	ADMUX = reg;

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Enable adc digital inputs
 * -------------------------------------------------------------------------- */

resultValue_t adcEnableDigitalInput(uint8 flagInputs)
{
	DIDR0 &= (~flagInputs);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Enable adc digital inputs
 * -------------------------------------------------------------------------- */

resultValue_t adcDisableDigitalInput(uint8 flagInputs)
{
	DIDR0 |= flagInputs;

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Sets the adc data presentation
 * -------------------------------------------------------------------------- */

resultValue_t adcDataPresentation(adcDataPresentation_t presentation)
{
	if(presentation == ADC_LEFT_ADJUSTED)
		setBit(ADMUX, ADLAR);
	else
		clrBit(ADMUX, ADLAR);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Enables the adc module
 * -------------------------------------------------------------------------- */

resultValue_t adcEnable(void)
{
	setBit(ADCSRA, ADEN);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Disables the adc module
 * -------------------------------------------------------------------------- */

resultValue_t adcDisable(void)
{
	clrBit(ADCSRA, ADEN);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Clears the adc interrupt request
 * -------------------------------------------------------------------------- */

resultValue_t adcClearInterruptRequest(void)
{
	setBit(ADCSRA, ADIF);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Enables the adc interrupt
 * -------------------------------------------------------------------------- */

resultValue_t adcActivateInterrupt(void)
{
	setBit(ADCSRA, ADIE);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Disables the adc interrupt
 * -------------------------------------------------------------------------- */

resultValue_t adcDeactivateInterrupt(void)
{
	clrBit(ADCSRA, ADIE);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Starts an adc conversion
 * -------------------------------------------------------------------------- */

resultValue_t adcStartConversion(void)
{
	setBit(ADCSRA, ADSC);

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Returns if an adc conversion is running
 * -------------------------------------------------------------------------- */

bool_t adcIsBusy(void)
{
	return isBitSet(ADCSRA, ADSC);
}

/* -----------------------------------------------------------------------------
 * Waits until the current conversion is finished
 * -------------------------------------------------------------------------- */

resultValue_t adcWaitUntilConversionFinish(void)
{
	waitUntilBitIsClear(ADCSRA, ADSC);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Configures the USART controller
 * -------------------------------------------------------------------------- */

resultValue_t usartConfig(usartMode_t mode, usartBaudRate_t baudRate, usartDataBits_t dataBits, usartParity_t parity, usartStopBits_t stopBits)
{
	uint8 reg1 = UCSR0A;
	uint8 reg2 = UCSR0B;
	uint8 reg3 = UCSR0C;
	uint8 modeAux = 0;
	uint8 aux8 = 0;
	uint64 aux64 = 0;

	// Clear errors
	reg1 &= ~((1 << FE0) | (1 << DOR0) | (1 << UPE0));

	// USART stop bits
	if(stopBits != USART_STOP_BIT_NO_CHANGE){
		switch(stopBits){
			case USART_STOP_BIT_SINGLE:	clrBit(reg3, USBS0);				break;
			case USART_STOP_BIT_DOUBLE:	setBit(reg3, USBS0);				break;
			default:					return RESULT_UNSUPPORTED_VALUE;	break;
		}
	}

	// USART parity bits
	if(parity != USART_PARITY_NO_CHANGE){
		reg3 &= ~(0x03 << UPM00);
		switch(parity){
			case USART_PARITY_NONE:		break;
			case USART_PARITY_EVEN:		reg3 |= (0x02 << UPM00);	break;
			case USART_PARITY_ODD:		reg3 |= (0x03 << UPM00);	break;
			case USART_PARITY_MARK:		return RESULT_USART_PARITY_UNSUPORTED;	break;
			case USART_PARITY_SPACE:	return RESULT_USART_PARITY_UNSUPORTED;	break;
			default:					return RESULT_USART_PARITY_UNSUPORTED;	break;
		}
	}

	// USART data bits
	if(dataBits != USART_DATA_BITS_NO_CHANGE){
		clrBit(reg2, UCSZ02);
		reg3 &= ~(0x03 << UCSZ00);
		switch(dataBits){
		case USART_DATA_BITS_5:
			break;
		case USART_DATA_BITS_6:
			reg3 |= (0x01 << UCSZ00);
			break;
		case USART_DATA_BITS_7:
			reg3 |= (0x02 << UCSZ00);
			break;
		case USART_DATA_BITS_8:
			reg3 |= (0x03 << UCSZ00);
			break;
		case USART_DATA_BITS_9:
			setBit(reg2, UCSZ02);
			reg3 |= (0x03 << UCSZ00);
			break;
		default:
			return RESULT_USART_PARITY_UNSUPORTED;
			break;
		}
	}

	// USART mode
	if(mode != USART_MODE_NO_CHANGE){
		modeAux = mode;
		reg3 &= ~(0x03 << UMSEL00);
		clrBit(reg1, U2X0);				// Single Speed
		clrBit(reg3, UCPOL0);			// Polarity
		switch(mode){
		case USART_MODE_NO_CHANGE:
			break;
		case USART_MODE_ASYNCHRONOUS:
			break;
		case USART_MODE_ASYNCHRONOUS_DOUBLE_SPEED:
			setBit(reg1, U2X0);			// Double Speed
			break;
		case USART_MODE_SYNCHRONOUS_XCK_FALLING:
			reg3 |= (1 << UMSEL00);		// Synchronous Mode
			break;
		case USART_MODE_SYNCHRONOUS_XCK_RISING:
			reg3 |= (1 << UMSEL00);		// Synchronous Mode
			break;
		case USART_MODE_SYNCHRONOUS_SPI:
			reg3 |= (3 << UMSEL00);		// Synchronous SPI Mode
			setBit(reg3, UCPOL0);		// Polarity
			break;
		}
	}else{
		aux8 = (0x03 & (UCSR0C >> UMSEL00));
		switch(aux8){
		case 0:	// Asynchronous mode
			if(isBitClr(UCSR0A, U2X0)){	// Single speed
				modeAux = USART_MODE_ASYNCHRONOUS;
			}else{						// Double speed
				modeAux = USART_MODE_ASYNCHRONOUS_DOUBLE_SPEED;
			}
			break;
		case 3: // Synchronous SPI mode
			modeAux = USART_MODE_SYNCHRONOUS_SPI;
			break;
		}
	}

	if(baudRate != USART_BAUD_NO_CHANGE){
		switch(modeAux){
		case USART_MODE_ASYNCHRONOUS:
			aux64 = (F_CPU / 16 / baudRate) - 1;
			break;
		case USART_MODE_ASYNCHRONOUS_DOUBLE_SPEED:
			aux64 = (F_CPU / 8 / baudRate) - 1;
			break;
		case USART_MODE_SYNCHRONOUS_SPI:
			aux64 = (F_CPU / 2 / baudRate) - 1;
			break;
		}
//		if((aux64 > (1.05 * baudRate)) || (aux64 < (0.95 * baudRate)))
//			return RESULT_USART_BAUD_RATE_UNSUPORTED;
		UBRR0H = 0x0F & (aux64 >> 8);
		UBRR0L = (0xFF & aux64);
	}

	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Enables USART reception module
 * -------------------------------------------------------------------------- */

resultValue_t usartEnableReceiver(void)
{
	setBit(UCSR0B, RXEN0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Disables USART reception module
 * -------------------------------------------------------------------------- */

resultValue_t usartDisableReceiver(void)
{
	clrBit(UCSR0B, RXEN0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Enables USART transmission module
 * -------------------------------------------------------------------------- */

resultValue_t usartEnableTransmitter(void)
{
	setBit(UCSR0B, TXEN0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Disables USART transmission module
 * -------------------------------------------------------------------------- */

resultValue_t usartDisableTransmitter(void)
{
	clrBit(UCSR0B, TXEN0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates USART reception complete interrupt
 * -------------------------------------------------------------------------- */

resultValue_t usartActivateReceptionCompleteInterrupt(void)
{
	setBit(UCSR0B, RXCIE0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates USART reception complete interrupt
 * -------------------------------------------------------------------------- */

resultValue_t usartDeactivateReceptionCompleteInterrupt(void)
{
	clrBit(UCSR0B, RXCIE0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates USART transmission complete interrupt
 * -------------------------------------------------------------------------- */

resultValue_t usartActivateTransmissionCompleteInterrupt(void)
{
	setBit(UCSR0B, TXCIE0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates USART transmission complete interrupt
 * -------------------------------------------------------------------------- */

resultValue_t usartDeactivateTransmissionCompleteInterrupt(void)
{
	clrBit(UCSR0B, TXCIE0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Activates USART buffer empty interrupt
 * -------------------------------------------------------------------------- */

resultValue_t usartActivateBufferEmptyInterrupt(void)
{
	setBit(UCSR0B, UDRIE0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Deactivates USART buffer empty interrupt
 * -------------------------------------------------------------------------- */

resultValue_t usartDeactivateBufferEmptyInterrupt(void)
{
	clrBit(UCSR0B, UDRIE0);
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Changes the std handlers to usart module
 * -------------------------------------------------------------------------- */

resultValue_t usartStdio(void)
{
	stdin = stdout = stderr = &usartStream;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Checks if a reception has been completed
 * -------------------------------------------------------------------------- */

bool_t usartIsReceptionComplete(void)
{
	return isBitSet(UCSR0A, RXC0);
}

/* -----------------------------------------------------------------------------
 * Checks if a transmission has been completed
 * -------------------------------------------------------------------------- */

bool_t usartIsTransmissionComplete(void)
{
	return isBitSet(UCSR0A, TXC0);
}

/* -----------------------------------------------------------------------------
 * Checks if the buffer is empty
 * -------------------------------------------------------------------------- */

bool_t usartIsBufferEmpty(void)
{
	return isBitSet(UCSR0A, UDRE0);
}

/* -----------------------------------------------------------------------------
 * Checks if an error occurred during transmission or reception
 * -------------------------------------------------------------------------- */

usartError_t usartCheckError(void)
{
	usartError_t error = 0;

	if(isBitSet(UCSR0A, FE0))
		error |= USART_FRAME_ERROR;
	if(isBitSet(UCSR0A, DOR0))
		error |= USART_PARITY_ERROR;
	if(isBitSet(UCSR0A, UPE0))
		error |= USART_BUFFER_OVERFLOW_ERROR;

	if(error == 0)
		return USART_OK;
	return error;
}

/* -----------------------------------------------------------------------------
 * Transmits data in 5, 6, 7 or 8 bits modes using the USART controller
 * -------------------------------------------------------------------------- */

resultValue_t usartTransmit(int8 data)
{
	while(!usartIsBufferEmpty())
		;	// Waits until last transmission ends
	UDR0 = data;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Transmits data in 9 bits mode using the USART controller
 * -------------------------------------------------------------------------- */

resultValue_t usartTransmit9bits(uint16 data)
{
	uint8 aux;

	while(!usartIsBufferEmpty())
		;	// Waits until last transmission ends
	aux = ((data & 0x100) >> 8);
	if(aux)
		setBit(UCSR0B, TXB80);
	else
		clrBit(UCSR0B, TXB80);
	UDR0 = (uint8)data;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Transmits data in 5, 6, 7 or 8 bits modes using the USART controller and
 * standard output heandler
 * -------------------------------------------------------------------------- */

resultValue_t usartTransmitStd(int8 data, FILE * stream)
{
	while(!usartIsBufferEmpty())
		;	// Waits until last transmission ends
	UDR0 = data;
	return RESULT_OK;
}

/* -----------------------------------------------------------------------------
 * Receives data in 5, 6, 7 or 8 bits modes using the USART controller
 * -------------------------------------------------------------------------- */

uint8 usartReceive(void)
{
	uint8 status;

	while(!usartIsReceptionComplete())
		;	// Waits until last reception ends
	status = UCSR0A;
	return UDR0;
}

/* -----------------------------------------------------------------------------
 * Receives data in 9 bits modes using the USART controller
 * -------------------------------------------------------------------------- */

uint16 usartReceive9bits(void)
{
	uint8 status;
	uint8 byteh;
	uint8 bytel;
	uint16 byte;

	while(!usartIsReceptionComplete())
		;	// Waits until last reception ends
	status = UCSR0A;
	byteh = UCSR0B;
	bytel = UDR0;

	byte = (uint16)(byteh & 0x02) << 7;
	byte |= bytel;
	return  byte;
}

/* -----------------------------------------------------------------------------
 * Receives data in 5, 6, 7 or 8 bits modes using the USART controller and
 * standard input heandler
 * -------------------------------------------------------------------------- */

int16 usartReceiveStd(FILE * stream)
{
	while(!usartIsReceptionComplete())
		;	// Waits until last reception ends
	return (int16)UDR0;
}

/* -----------------------------------------------------------------------------
 * Clears the receptor data buffer
 * -------------------------------------------------------------------------- */

void usartClearReceptionBuffer(void)
{
	uint8 aux;
	while(usartIsReceptionComplete())
		aux = UDR0;
	return;
}




















/* -----------------------------------------------------------------------------
 * Adds data to the reception buffer. The function has NO CONTROL of lost data.
 * -------------------------------------------------------------------------- */

void usartAddDataToReceiverBuffer(uint8 data)
{
	if(((usartReceiverBufferNextWrite + 1) % USART_RECEIVER_BUFFER_SIZE) != usartReceiverBufferNextRead){
		usartReceiverBuffer[usartReceiverBufferNextWrite] = data;
		usartReceiverBufferNextWrite = (usartReceiverBufferNextWrite + 1) % USART_RECEIVER_BUFFER_SIZE;
		usartReceiverBufferLength++;
	}
	return;
}

/* -----------------------------------------------------------------------------
 * Gets data from the reception buffer. If the buffer is empty, the last
 * retrieved data will be returned and the pointer will not be changed. The
 * usartIsReceiverBufferEmpty() function must be called to check if there is new
 * data in the buffer.
 * -------------------------------------------------------------------------- */

uint8 usartGetDataFromReceiverBuffer(void)
{
	uint8 data = usartReceiverBuffer[usartReceiverBufferNextRead];
	if(usartReceiverBufferLength > 0){
		usartReceiverBufferNextRead = (usartReceiverBufferNextRead + 1) % USART_RECEIVER_BUFFER_SIZE;
		usartReceiverBufferLength--;
	}
	return data;
}

/* -----------------------------------------------------------------------------
 * Verifies if there is new data in the receiver buffer. Must be called before
 * reading the buffer.
 * -------------------------------------------------------------------------- */

uint8 usartIsReceiverBufferEmpty(void)
{
	if(usartReceiverBufferLength == 0)
		return TRUE;
	return FALSE;
}

/* -----------------------------------------------------------------------------
 * Returns the current baud rate.
 * -------------------------------------------------------------------------- */

 uint32 usartGetCurrentBaudRate(void)
 {
	uint32 aux32 = 0;

//	switch(usartConfiguration.mode){
//	case 0:
//		aux32 = F_CPU / 16 / (UBRR0 + 1);
//		break;
//	case 1:
//		aux32 = F_CPU / 8 / (UBRR0 + 1);
//		break;
//	case 2:
//		aux32 = F_CPU / 2 / (UBRR0 + 1);
//		break;
//	}
	return aux32;
 }



















/* -----------------------------------------------------------------------------
 * Configures the SPI controller
 * -------------------------------------------------------------------------- */

void spiInit(void)
{
/*
	if(spiConfiguration.masterSlave){
		setBit(SPI_DDR, SPI_MOSI);
		setBit(SPI_DDR, SPI_SCK);
		clrBit(SPI_DDR, SPI_MISO);
	}else{
		clrBit(SPI_DDR, SPI_MOSI);
		clrBit(SPI_DDR, SPI_SCK);
		setBit(SPI_DDR, SPI_MISO);
	}
	if(spiConfiguration.doubleSpeed)
		setBit(SPSR, SPI2X);
	else
		clrBit(SPSR, SPI2X);
	SPCR =	(spiConfiguration.interruptEnabled << SPIE) | 
			(spiConfiguration.enabled << SPE) | 
			(spiConfiguration.msbLsb << DORD) | 
			(spiConfiguration.ma	sterSlave << MSTR) | 
			(spiConfiguration.sckIdleValue << CPOL) | 
			(spiConfiguration.leadingTrailingEdge << CPHA) | 
			(spiConfiguration.clockPrescaler & 0x03);

*/
	return;
}

/* -----------------------------------------------------------------------------
 * Transmit data at the SPI bus in master mode
 * -------------------------------------------------------------------------- */

uint8 spiMasterTransmit(uint8 data)
{
	SPDR = data;
	waitUntilBitIsClear(SPSR, SPIF);
	return SPDR;
}

/* -----------------------------------------------------------------------------
 * Receives data from the SPI bus
 * -------------------------------------------------------------------------- */

uint8 spiSlaveTransmit(void)
{
	waitUntilBitIsClear(SPSR, SPIF);
	return SPDR;
}

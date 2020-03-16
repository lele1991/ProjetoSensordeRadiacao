/* -----------------------------------------------------------------------------
 * Project:			GPDSE AVR8 Library
 * File:			ATmega328.h
 * Module:			ATmega328 basic interface
 * Author:			Leandro Schwarz
 * Version:			12.2
 * Last edition:	2016-01-15
 * -------------------------------------------------------------------------- */

#ifndef __ATMEGA328_H
#define __ATMEGA328_H 122

// -----------------------------------------------------------------------------
// Microcontroller constant identifier -----------------------------------------

#define MCU	_ATMEGA328_

// -----------------------------------------------------------------------------
// Header files ----------------------------------------------------------------

#include "globalDefines.h"
#if __GLOBALDEFINES_H < 123
	#error Error 100 - The defintion file is outdated (globalDefines must be version 12.3 or greater).
#endif
#include <avr/io.h>
#include <util/atomic.h>
#include <stdio.h>

// -----------------------------------------------------------------------------
// Some definitions that seemed to be forgotten in avr/io.h ---------------------

#ifndef SPMEN
	#define SPMEN	0
#endif

// -----------------------------------------------------------------------------
// New data types --------------------------------------------------------------

#define USART_RECEIVER_BUFFER_SIZE 50UL
extern FILE usartStream;
extern uint8 usartReceiverBuffer[USART_RECEIVER_BUFFER_SIZE];
extern uint8 usartReceiverBufferNextRead;
extern uint8 usartReceiverBufferNextWrite;
extern uint8 usartReceiverBufferLength;

// -----------------------------------------------------------------------------
// Status register -------------------------------------------------------------

void	globalInterruptEnable(void);
void	globalInterruptDisable(void);

// -----------------------------------------------------------------------------
// System clock prescaler ------------------------------------------------------

resultValue_t	systemClockSetPrescaler(systemPrescalerValue_t prescaler);

// -----------------------------------------------------------------------------
// EEPROM memory ---------------------------------------------------------------

#define EEPROM_ADDRESS_MASK	0x03FF

void	eepromSetOperationMode(eepromMode_t mode);
void	eepromReadyActivateInterrupt(void);
void	eepromReadyDeactivateInterrupt(void);
uint8	eepromRead(uint16 address);
void	eepromWrite(uint16 address, uint8 data);

// -----------------------------------------------------------------------------
// Pull-Up ---------------------------------------------------------------------

void	globalPullUpDisable(void);
void	globalPullUpEnable(void);

// -----------------------------------------------------------------------------
// External interrupts ---------------------------------------------------------

// INT0
resultValue_t	int0Config(portMode_t port, senseMode_t sense);
void			int0ActivateInterrupt(void);
void			int0DeactivateInterrupt(void);
void			int0ClearInterruptRequest(void);

// INT1
resultValue_t	int1Config(portMode_t port, senseMode_t sense);
void			int1ActivateInterrupt(void);
void			int1DeactivateInterrupt(void);
void			int1ClearInterruptRequest(void);

// PCINT7-0
void			pcint7_0Enable(void);
void			pcint7_0Disable(void);
void			pcint7_0ClearInterruptRequest(void);
resultValue_t	pcint0ActivateInterrupt(portMode_t port);
void			pcint0DeactivateInterrupt(void);
resultValue_t	pcint1ActivateInterrupt(portMode_t port);
void			pcint1DeactivateInterrupt(void);
resultValue_t	pcint2ActivateInterrupt(portMode_t port);
void			pcint2DeactivateInterrupt(void);
resultValue_t	pcint3ActivateInterrupt(portMode_t port);
void			pcint3DeactivateInterrupt(void);
resultValue_t	pcint4ActivateInterrupt(portMode_t port);
void			pcint4DeactivateInterrupt(void);
resultValue_t	pcint5ActivateInterrupt(portMode_t port);
void			pcint5DeactivateInterrupt(void);
resultValue_t	pcint6ActivateInterrupt(portMode_t port);
void			pcint6DeactivateInterrupt(void);
resultValue_t	pcint7ActivateInterrupt(portMode_t port);
void			pcint7DeactivateInterrupt(void);

// PCINT14-8
void			pcint14_8Enable(void);
void			pcint14_8Disable(void);
void			pcint14_8ClearInterruptRequest(void);
resultValue_t	pcint8ActivateInterrupt(portMode_t port);
void			pcint8DeactivateInterrupt(void);
resultValue_t	pcint9ActivateInterrupt(portMode_t port);
void			pcint9DeactivateInterrupt(void);
resultValue_t	pcint10ActivateInterrupt(portMode_t port);
void			pcint10DeactivateInterrupt(void);
resultValue_t	pcint11ActivateInterrupt(portMode_t port);
void			pcint11DeactivateInterrupt(void);
resultValue_t	pcint12ActivateInterrupt(portMode_t port);
void			pcint12DeactivateInterrupt(void);
resultValue_t	pcint13ActivateInterrupt(portMode_t port);
void			pcint13DeactivateInterrupt(void);
resultValue_t	pcint14ActivateInterrupt(portMode_t port);
void			pcint14DeactivateInterrupt(void);

// PCINT23-16
void			pcint23_16Enable(void);
void			pcint23_16Disable(void);
void			pcint23_16ClearInterruptRequest(void);
resultValue_t	pcint16ActivateInterrupt(portMode_t port);
void			pcint16DeactivateInterrupt(void);
resultValue_t	pcint17ActivateInterrupt(portMode_t port);
void			pcint17DeactivateInterrupt(void);
resultValue_t	pcint18ActivateInterrupt(portMode_t port);
void			pcint18DeactivateInterrupt(void);
resultValue_t	pcint19ActivateInterrupt(portMode_t port);
void			pcint19DeactivateInterrupt(void);
resultValue_t	pcint20ActivateInterrupt(portMode_t port);
void			pcint20DeactivateInterrupt(void);
resultValue_t	pcint21ActivateInterrupt(portMode_t port);
void			pcint21DeactivateInterrupt(void);
resultValue_t	pcint22ActivateInterrupt(portMode_t port);
void			pcint22DeactivateInterrupt(void);
resultValue_t	pcint23ActivateInterrupt(portMode_t port);
void			pcint23DeactivateInterrupt(void);


// -----------------------------------------------------------------------------
// Timer/counter 0 -------------------------------------------------------------

resultValue_t	timer0Config(timerModeA_t mode, timerPrescalerValueA_t prescaler);
resultValue_t	timer0OutputConfig(timerOutput_t compA, timerOutput_t compB);
void			timer0ActivateOverflowInterrupt(void);
void			timer0DeactivateOverflowInterrupt(void);
void			timer0ClearOverflowInterruptRequest(void);
void			timer0ActivateCompareAInterrupt(void);
void			timer0DeactivateCompareAInterrupt(void);
void			timer0ClearCompareAInterruptRequest(void);
void			timer0ActivateCompareBInterrupt(void);
void			timer0DeactivateCompareBInterrupt(void);
void			timer0ClearCompareBInterruptRequest(void);
void			timer0ForceCompareA(void);
void			timer0ForceCompareB(void);
void			timer0SetCounterValue(uint8 value);
uint8			timer0GetCounterValue(void);
void			timer0SetCompareAValue(uint8 value);
uint8			timer0GetCompareAValue(void);
void			timer0SetCompareBValue(uint8 value);
uint8			timer0GetCompareBValue(void);

// -----------------------------------------------------------------------------
// Timer/counter 1 -------------------------------------------------------------

resultValue_t	timer1Config(timerModeB_t mode, timerPrescalerValueA_t prescaler);
resultValue_t	timer1OutputConfig(timerOutput_t compA, timerOutput_t compB);
resultValue_t	timer1InputCaptureNoiseCancelerConfig(timerInputCaptureNoiseCanceler_t mode);
resultValue_t	timer1ActivateOverflowInterrupt(void);
resultValue_t	timer1DeactivateOverflowInterrupt(void);
resultValue_t	timer1ClearOverflowInterruptRequest(void);
resultValue_t	timer1ActivateCompareAInterrupt(void);
resultValue_t	timer1DeactivateCompareAInterrupt(void);
resultValue_t	timer1ClearCompareAInterruptRequest(void);
resultValue_t	timer1ActivateCompareBInterrupt(void);
resultValue_t	timer1DeactivateCompareBInterrupt(void);
resultValue_t	timer1ClearCompareBInterruptRequest(void);
resultValue_t	timer1ActivateInputCaptureInterrupt(void);
resultValue_t	timer1DeactivateInputCaptureInterrupt(void);
resultValue_t	timer1ClearInputCaptureInterruptRequest(void);
resultValue_t	timer1ForceCompareA(void);
resultValue_t	timer1ForceCompareB(void);
resultValue_t	timer1SetCounterValue(uint16 value);
uint16			timer1GetCounterValue(void);
resultValue_t	timer1SetCompareAValue(uint16 value);
uint16			timer1GetCompareAValue(void);
resultValue_t	timer1SetCompareBValue(uint16 value);
uint16			timer1GetCompareBValue(void);
resultValue_t	timer1SetInputCaptureValue(uint16 value);
uint16			timer1GetInputCaptureValue(void);

// -----------------------------------------------------------------------------
// Timer/counter 2 -------------------------------------------------------------

resultValue_t	timer2Config(timerModeA_t mode, timerPrescalerValueB_t prescaler);
resultValue_t	timer2OutputConfig(timerOutput_t compA, timerOutput_t compB);
resultValue_t	timer2ActivateOverflowInterrupt(void);
resultValue_t	timer2DeactivateOverflowInterrupt(void);
resultValue_t	timer2ClearOverflowInterruptRequest(void);
resultValue_t	timer2ActivateCompareAInterrupt(void);
resultValue_t	timer2DeactivateCompareAInterrupt(void);
resultValue_t	timer2ClearCompareAInterruptRequest(void);
resultValue_t	timer2ActivateCompareBInterrupt(void);
resultValue_t	timer2DeactivateCompareBInterrupt(void);
resultValue_t	timer2ClearCompareBInterruptRequest(void);
resultValue_t	timer2ForceCompareA(void);
resultValue_t	timer2ForceCompareB(void);
resultValue_t	timer2SetCounterValue(uint8 value);
uint8			timer2GetCounterValue(void);
resultValue_t	timer2SetCompareAValue(uint8 value);
uint8			timer2GetCompareAValue(void);
resultValue_t	timer2SetCompareBValue(uint8 value);
uint8			timer2GetCompareBValue(void);
// ASSR register
// GTCCR register


// -----------------------------------------------------------------------------
// Serial Peripheral Interface -------------------------------------------------

#define SPI_DDR						DDRB
#define SPI_MOSI					PB3
#define SPI_MISO					PB4
#define SPI_SCK						PB5

#define spiMaster()					spiConfiguration.masterSlave = 1
#define spiSlave()					spiConfiguration.masterSlave = 0
#define spiEnable()					spiConfiguration.enabled = 1
#define spiDisable()				spiConfiguration.enabled = 0
#define spiEnableInterrupt()		spiConfiguration.interruptEnabled = 1
#define spiDisableInterrupt()		spiConfiguration.interruptEnabled = 0
#define spiMSBFirst()				spiConfiguration.msbLsb = 0
#define spiLSBFirst()				spiConfiguration.msbLsb = 1
#define spiSCKIdleLow()				spiConfiguration.sckIdleValue = 0
#define spiSCKIdleHigh()			spiConfiguration.sckIdleValue = 1
#define spiSampleLeadingEdge()		spiConfiguration.leadingTrailingEdge = 0
#define spiSampleTrailingEdge()		spiConfiguration.leadingTrailingEdge = 1
#define spiEnableDoubleSpeed()		spiConfiguration.doubleSpeed = 1
#define spiDisableDoubleSpeed()		spiConfiguration.doubleSpeed = 0
#define spiClockPrescaler2()		spiConfiguration.clockPrescaler = 4
#define spiClockPrescaler4()		spiConfiguration.clockPrescaler = 0
#define spiClockPrescaler8()		spiConfiguration.clockPrescaler = 5
#define spiClockPrescaler16()		spiConfiguration.clockPrescaler = 1
#define spiClockPrescaler32()		spiConfiguration.clockPrescaler = 6
#define spiClockPrescaler64()		spiConfiguration.clockPrescaler = 2
#define spiClockPrescaler128()		spiConfiguration.clockPrescaler = 3

void	spiInit(void);
uint8	spiMasterTransmit(uint8 data);
uint8	spiSlaveTransmit(void);

// -----------------------------------------------------------------------------
// Analog/Digital Converter ----------------------------------------------------

#define ADC_INTERNAL_REFERENCE_VALUE_V	1.1
#define ADC_INTERNAL_REFERENCE_VALUE_MV	1100
#define DISABLE_INPUT_0	0x01
#define DISABLE_INPUT_1	0x02
#define DISABLE_INPUT_2	0x04
#define DISABLE_INPUT_3	0x08
#define DISABLE_INPUT_4	0x10
#define DISABLE_INPUT_5	0x20
#define DISABLE_INPUT_6	0x40
#define DISABLE_INPUT_7	0x80

resultValue_t	adcConfig(adcMode_t mode, adcReference_t reference, adcPrescaler_t  prescaler);
resultValue_t	adcSelectChannel(adcChannel_t channel);
resultValue_t	adcEnableDigitalInput(uint8 flagInputs);
resultValue_t	adcDisableDigitalInput(uint8 flagInputs);
resultValue_t	adcDataPresentation(adcDataPresentation_t presentation);
resultValue_t	adcEnable(void);
resultValue_t	adcDisable(void);
resultValue_t	adcClearInterruptRequest(void);
resultValue_t	adcActivateInterrupt(void);
resultValue_t	adcDeactivateInterrupt(void);
resultValue_t	adcStartConversion(void);
bool_t			adcIsBusy(void);
resultValue_t	adcWaitUntilConversionFinish(void);

// -----------------------------------------------------------------------------
// USART0 ----------------------------------------------------------------------

#define USART_RECEIVER_BUFFER_FUNCTION_HANDLER			ISR(USART_RX_vect){usartAddDataToReceiverBuffer(usartReceive());return;}

resultValue_t	usartConfig(usartMode_t mode, usartBaudRate_t baudRate, usartDataBits_t dataBits, usartParity_t parity, usartStopBits_t stopBits);
resultValue_t	usartEnableReceiver(void);
resultValue_t	usartDisableReceiver(void);
resultValue_t	usartEnableTransmitter(void);
resultValue_t	usartDisableTransmitter(void);
resultValue_t	usartActivateReceptionCompleteInterrupt(void);
resultValue_t	usartDeactivateReceptionCompleteInterrupt(void);
resultValue_t	usartActivateTransmissionCompleteInterrupt(void);
resultValue_t	usartDeactivateTransmissionCompleteInterrupt(void);
resultValue_t	usartActivateBufferEmptyInterrupt(void);
resultValue_t	usartDeactivateBufferEmptyInterrupt(void);
resultValue_t	usartStdio(void);
bool_t			usartIsReceptionComplete(void);
bool_t			usartIsTransmissionComplete(void);
bool_t			usartIsBufferEmpty(void);
usartError_t	usartCheckError(void);
resultValue_t	usartTransmit(int8 data);
resultValue_t	usartTransmit9bits(uint16 data);
resultValue_t	usartTransmitStd(int8 data, FILE * stream);
uint8			usartReceive(void);
uint16			usartReceive9bits(void);
int16			usartReceiveStd(FILE * stream);
void			usartClearReceptionBuffer(void);

void usartAddDataToReceiverBuffer(uint8 data);
uint8 usartGetDataFromReceiverBuffer(void);
uint8 usartIsReceiverBufferEmpty(void);
uint32 usartGetCurrentBaudRate(void);

#endif

#include <stdint.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "lib/bits.h"
#include "lib/avr_gpio.h"
#include "lib/avr_timer.h"
#include "lib/avr_extirq.h"
#include "sensor.h"
#include <util/delay.h>
#include "lib/avr_adc.h"
#include "lib/avr_usart.h"
#include "lib/avr_spi.h"
#include "globalDefines.h"
#include "ATmega328.h"
#include "ff.h"
#include "sensor.h"
#include "ds1307.h"
#include <string.h>


#define HORA		15
#define MINUTO		35
#define SEGUNDO		00

#define ANO			19
#define MES			6
#define DIA			19
#define DIA_SEMANA	4


#define LED_PIN	PB0
// AD0-a0(sensor radiacao)

//cartao SD
// CS - Pino 4
//MOSI - PINO 11
//SCK - PINO 13
//MISO - PINO 12

//rtc
//SCL - PINO 28
//SDA - PINO 27
//DS - PINO 
 
// System definitions ----------------------------------------------------------
#define DRV_MMC

int main(){
	/* Inicializa o converor AD (sensor radiacao)*/
	adcEtimer_init();

	//cartao

	// Variable declaration
	FRESULT res;
	FATFS card;
	FIL file;
	char string[64];

	uint16_t bytesWritten, result=0, n=0;

	//sensor efeito hall
	uint16_t AD_hall=0;

	//tensao
	uint32_t tensao_res=0, potencia_res=0, corrente_res=0, pot1=0, pot2=0;
	uint16_t AD_radiacao=0;
	memset(string, 0, sizeof(string));

	// TWI Init
	twiMasterInit(10000);

	// UART configuration
	usartConfig(USART_MODE_ASYNCHRONOUS, USART_BAUD_9600, USART_DATA_BITS_8, USART_PARITY_NONE, USART_STOP_BIT_SINGLE);
	usartEnableReceiver();
	usartEnableTransmitter();
	usartStdio();
	printf("SD Card Example\n \r");

	// Enable Global Interrupts
	sei();

	// RTC Configuration
	ds1307SetControl(DS1307_COUNTING_RESUME, DS1307_CLOCK_1HZ, DS1307_FORMAT_24_HOURS);

	//comentar depois de gravar
	//ds1307SetDate(ANO, MES, DIA, DIA_SEMANA);
	//ds1307SetTime(HORA, MINUTO, SEGUNDO, DS1307_24);
	// Mounting SD card

	res = f_mount(0, &card);
	if(res != FR_OK){
		printf("->SD card not mounted => error = %d\n \r", res);
	}else{
		printf("->SD card successfully mounted\n \r");
	}

	ds1307SetControl(DS1307_COUNTING_NO_CHANGE,DS1307_CLOCK_NO_CHANGE, DS1307_FORMAT_24_HOURS );


	//printf("antes res ");

	res = f_open(&file, "Radiacao.csv", FA_WRITE | FA_CREATE_ALWAYS);
	//printf("depois res");

	if(res != FR_OK){
		printf("->File not created => error = %d \n \r", res);
	}
	else{
		printf("->File created successfully \n \r ");

	}


	while(1){
		//sensor efeito hall
		_delay_ms(10000); // a cada 30 segundos
		ds1307GetTime(&(dados_t.tempo_t.hora),&(dados_t.tempo_t.minuto) ,&(dados_t.tempo_t.segundo),&(dados_t.tempo_t.am_pm)); // define  funfa??

		AD_hall = (dados_t.dado_tensao>>3);
		printf("ad_hall: %d\n", AD_hall);
		AD_radiacao = (dados_t.dado_radiacao>>3);
		printf("ad_rad: %d\n", AD_radiacao);

		n = snprintf(string, 64, "%d; %d; %d:%d:%d\n", AD_hall, AD_radiacao, dados_t.tempo_t.hora, dados_t.tempo_t.minuto, dados_t.tempo_t.segundo);
		printf("SNPRINTF: %s\n", string);
		result = f_write(&file, string, strlen(string), &bytesWritten);
		//3.19 resistor que esta medindo em cima 10+10 em serie

		tensao_res =( AD_hall*5)/1024;
		corrente_res = (tensao_res*10)/3.19; // multiplicada por 10
		potencia_res = (10*corrente_res*corrente_res)+(10*corrente_res*corrente_res)+(3.19*corrente_res*corrente_res);
		pot1 = potencia_res/10;
		pot2 = potencia_res%10;

		if(result!=0){
			printf("fr_ok = %d",result);
		}
		f_sync(&file);
	}
}




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
#include "ds1307.h"


volatile uint16_t valor_adc = 0;
volatile uint16_t i = 0, sum = 0, n=0, flag=0,sen_hall=0;
unsigned int x[32];

volatile uint16_t sum_ADC0 = 0;
volatile uint16_t sum_ADC1 = 0;

volatile uint16_t ADC0[8] = {0};
volatile uint16_t ADC1[8] = {0};

void adcEtimer_init(){
	/* Acesso indireto por struct e bit field: com avr_timer.h */
		TIMER_0->TCCRA = 0;
		TIMER_0->TCCRB = SET(CS02) | SET(CS00);
		TIMER_IRQS->TC0.BITS.TOIE = 1;

	/* Ref externa no pino AVCC com capacitor de 100n em VREF.
		 * Habiltiação apenas no Canal 0 */
	ADCS->AD_MUX = SET(REFS0);
	/* Habilita AD:
	 * Conversão contínua
	 * IRQ ativo
	 * Prescaler de 128 */
	ADCS->ADC_SRA = SET(ADEN)  |							//ADC Enable
					SET(ADSC)  | 							// ADC Start conversion
					SET(ADATE) |							// ADC Auto Trigger
					SET(ADPS0) | SET(ADPS1) | SET(ADPS2) |	//ADPS[0..2] AD Prescaler selection
					SET(ADIE); 								//AD IRQ ENABLE

	/* Auto trigger in timer0 overflow */
	ADCS->ADC_SRB = SET(ADTS2);

	/* Desabilita hardware digital de PC0 */
	ADCS->DIDr0.BITS.ADC0 = 1;

}


ISR(ADC_vect)
{
	uint16_t valor_adc = 0;
		static uint8_t i_adc0=0;
		static uint8_t i_adc1=0;

		/* Lê o valor do conversor AD na interrupção:
		 * ADC é de 10 bits, logo valor_adc deve ser
		 * de 16 bits
		 */
		valor_adc = ADC;

		if (!TST_BIT(ADCS->AD_MUX, 0)){
			//ADC0
			sum_ADC0 = sum_ADC0 - ADC0[i_adc0] + valor_adc;
			ADC0[i_adc0] = valor_adc;
			i_adc0++;
			i_adc0 &= 0x7;
			//printf("ADC0=%d \n \r",ADC0[i_adc0]);

		}
		else {
			sum_ADC1 = sum_ADC1 - ADC1[i_adc1] + valor_adc;
			ADC1[i_adc1] = valor_adc;
			i_adc1++;
			i_adc1 &= 0x7;
			//printf("ADC1=%d \n \r",ADC1[i_adc1]);

		}
		dados_t.dado_radiacao = sum_ADC0;// sensor radiaçao->string
		dados_t.dado_tensao=sum_ADC1;

		GPIO_CplBit(GPIO_B, 1);
		CPL_BIT(ADCS->AD_MUX, 0);
}

/* Quando habilitado IRQ de overflow no timer 0*/
ISR(TIMER0_OVF_vect){
	GPIO_CplBit(GPIO_B, 0);
}


volatile uint16_t get_flag(){
	return flag;
}
/*
void sensor_handler(){
	uint16_t soma=0,potencia=0,tensao=0,corrente,conversao_adc_tensao;
	soma=sum;

		printf("sensor handler \n \r");

		dados_t.dado_radiacao = sum_ADC0>>3;// sensor radiaçao->string
		ds1307GetTime(&(dados_t.tempo_t.hora),&(dados_t.tempo_t.minuto) ,&(dados_t.tempo_t.segundo),&(dados_t.tempo_t.am_pm)); // define  funfa??

		//66mv/A
		//conversao_adc_tensao=(5*(ADC1>>3))/1024;  // converte adc para tensao

		//printf("adc_tensao: %d \n \r",conversao_adc_tensao);

		dados_t.dado_corrente=sum_ADC1>>3;

}*/

void hardware_init(){

	/* Acesso direto: PB0 como saída demais como entrada
	 * como no assembly */
	DDRB = 0b00000001;
	/* Pull ups */
	PORTB = 0b00000010;
	//_delay_ms(500);
	/* PORTB como saída */
	GPIO_B->DDR = 0xff;
}


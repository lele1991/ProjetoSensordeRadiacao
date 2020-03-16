#ifndef SENSOR_H_
#define SENSOR_H_

#include "globalDefines.h"
#include "ds1307.h"
void adcEtimer_init();
volatile uint16_t get_flag();
//void sensor_handler();
void hardware_init();

struct{
	uint16 dado_radiacao;
	uint16 dado_potencia;
	uint16 dado_tensao;
	uint16 dado_corrente;
	struct{
		uint8_t hora;
		uint8_t minuto;
		uint8_t segundo;
		ds1307TimeAbbreviation_t am_pm;

	}tempo_t;
}dados_t;

#endif /* SENSOR_H_ */

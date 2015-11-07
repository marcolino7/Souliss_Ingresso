/***************************************************************************
	--- Souliss ---

        Controllo Ingresso Temperatura e Consumi
		PIN
		-  0		RX Usart
		-  1		TX Usart
		-  2
		-  3		PIN_220_1		220V Detect 1
		-  4		PIN_220_2		220V Detect 2
		-  5
		-  6
		-  7
		-  8
		-  9
		- 10
		- 11		PIN_TEMP		DS18B20
		- 12		PIN_RELE_1		Relè 1
		- 13
		- A0-14		PIN_CT_1		CT Sens 1
		- A1-15		PIN_CT_2		CT Sens 2
		- A2-16		PIN_CT_3		CT Sens 3
		- A3-17		PIN_AUX_1		Ingresso AUX 1
		- A4-18		PIN_AUX_2		Ingresso AUX 2
		- A5-19		PIN_RELE_2		Relè 2

		Ciseco Remote Programming
		Node Address = 04
		Channel Offset = 3
		BaudRate = 57600

****************************************************************************/

#define USARTBAUDRATE_INSKETCH
#define	USART_BAUD57k6			1
#define USART_BAUD115k2			0
#define USART_DEBUG  			0

#include "bconf/StandardArduino.h"			// Use a standard Arduino
#include "conf/usart.h"

#include "Souliss.h"
#include "OneWire.h"
#include "DallasTemperature.h"
#include "EmonLib.h"


// network addresses
#define myvNet_address		0xCE04
#define myvNet_subnet		0xFF00
#define myvNet_supern		0x0000

#define HDEADBAND	  0.50	
#define DEADBAND      0.05			//Se la variazione è superio del 5% aggiorno
#define LDEADBAND   0.01			//Se la variazione è superio del 1% aggiorno
#define NODEADBAND	  0				//Se la variazione è superio del 0,1% aggiorno

//----------- Define Typical
#define T_LUCE		0	//Tipico per Gestire l'illuminazione in corridoio con step relay
#define	T_TEMP		1	//DS18B20
#define T_CT_1_W	3	//CT Sens 1 Watt
#define T_CT_1_A	5	//CT Sens 1 Ampere
#define T_220		7	//Presenza 220Volt
#define T_RELE_2	8	//T11


//------------Define PIN
#define PIN_220_1	3	//220V Detect 1
#define PIN_220_2	4	//220V Detect 2	
#define	PIN_TEMP	11	//DS18B20
#define	PIN_RELE_1	12	//Relè 1
#define PIN_CT_1	A0	//CT Sens 1 - Digital 14
#define	PIN_CT_2	A1	//CT Sens 2 - Digital 15
#define	PIN_CT_3	A2	//CT Sens 3 - Digital 16
#define	PIN_AUX_1	A3	//Ingresso AUX 1 - Analog A3 Digital 17
#define	PIN_AUX_2	A4	//Ingresso AUX 2 - Analog A4 Digital 18
#define	PIN_RELE_2	19	//Relè 2 - Analog A5


//Sensore di Corrente 1
	//Calibrazione
const int volt_1 = 230;
const float ct_calib_1 = 29.7;
	//Variabili
float Irms_1 = 0;

// Create an Emon instance
EnergyMonitor emon1;

//Sonda Temperatura
#define TEMPERATURE_PRECISION	9	
float t_amb;
OneWire oneWire(PIN_TEMP);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

// flag 
//U8 data_chg = 1;


void setup()
{	
	Souliss_SetAddress(myvNet_address, myvNet_subnet, myvNet_supern);
	
	//Imposto i Pin Mode
	pinMode(PIN_220_1,INPUT);		//Stato Luce
	pinMode(PIN_RELE_1, OUTPUT);	//Relè 1 Luce
	pinMode(PIN_220_2, INPUT);		//Verifica Presenza 220Volt

	pinMode(PIN_RELE_2, OUTPUT);	//Relè 2

	//Definisco i Tipici
	Souliss_SetT18(memory_map, T_LUCE);			//Tipico per gestire le luci all'ingresso
	Souliss_SetT52(memory_map, T_TEMP);			//Tipico per leggere la temperatura
	Souliss_SetT57(memory_map, T_CT_1_W);		//Imposto il tipico per contenere i watt
	Souliss_SetT56(memory_map, T_CT_1_A);		//Imposto il tipico per contenere gli ampere
	Souliss_SetT13(memory_map, T_220);			//Imposto il tipico per la presenza di corrente

	Souliss_SetT11(memory_map, T_RELE_2);		//Relè 2
	
	//initialize Emon instance
	emon1.current(PIN_CT_1, ct_calib_1);
	
	//Imposto la sonda DS18B20
	sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
	sensors.getAddress(insideThermometer, 0);

}

void loop()
{
	EXECUTEFAST() {						
		UPDATEFAST();
		FAST_50ms() {
			// Legge lo stato e Invia il comando al Relè delle luci dal T18
			Souliss_DigOut(PIN_RELE_1, Souliss_T1n_ToggleCmd, memory_map,T_LUCE);
			Souliss_LowDigIn2State(PIN_220_1, Souliss_T1n_OnFeedback, Souliss_T1n_OffFeedback,memory_map, T_LUCE);
			
			//Invia il comando al T11 Ausiliario
			Souliss_DigOut(PIN_RELE_2, Souliss_T1n_Coil, memory_map, T_RELE_2);

			//Legge la presenza di corrente dal T13
			Souliss_LowDigIn2State(PIN_220_2, Souliss_T1n_OnCmd, Souliss_T1n_OffCmd, memory_map, T_220);


		}

		FAST_70ms() {
			Souliss_Logic_T18(memory_map, T_LUCE, &data_changed);
			Souliss_Logic_T13(memory_map, T_220, &data_changed);
			Souliss_Logic_T11(memory_map, T_RELE_2, &data_changed);
		}

		FAST_90ms() {
			//processa Corrente e Potenza
			Souliss_Logic_T56(memory_map, T_CT_1_A, LDEADBAND, &data_changed);
			Souliss_Logic_T57(memory_map, T_CT_1_W, LDEADBAND, &data_changed);
		}

		FAST_110ms() {
			//Processa la logica per la temperatura
			Souliss_Logic_T52(memory_map, T_TEMP, NODEADBAND, &data_changed);
		}
		
		FAST_210ms() {
			PowerRead_1();
		}
		
		FAST_510ms() {
		}
		
		FAST_1110ms() {
		}

        FAST_2110ms() {
			DSRead();	//Routine per leggere il valore della sonda e importarlo in Souliss
        }
		FAST_PeerComms();
}
	
	EXECUTESLOW() {
		UPDATESLOW();
		SLOW_10s() {
		}
	}		
}

void DSRead() {
	//Leggo la sonda DHT
	sensors.requestTemperatures();
	float t_amb = sensors.getTempC(insideThermometer);
	Souliss_ImportAnalog(memory_map, T_TEMP, &t_amb);
}

void PowerRead_1() {
	//Leggo la potenza dalla sonda CT
	float watt = 0;
	Irms_1 = emon1.calcIrms(1480);
	Souliss_ImportAnalog(memory_map, T_CT_1_A, &Irms_1);
	watt = Irms_1*volt_1;
	Souliss_ImportAnalog(memory_map, T_CT_1_W, &watt);
	
}
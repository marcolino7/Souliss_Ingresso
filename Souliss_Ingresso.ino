/**************************************************************************
	--- Souliss ---
    -- #BUILD:4 --

        Controllo Ingresso


		'Ciseco Remote Programming
		'Node Address = 03
		'Channel Offset = 3
		'BaudRate = 57600

***************************************************************************/
//		0x43		Arduino with USART

#define  BOARDTYPE_INSKETCH
#define  QC_BOARDTYPE			0x43    //		0x43		Arduino with USART

#define  GATEWAYTYPE_INSKETCH
#define	QC_GATEWAYTYPE			0x00    //0x00 None

#define INTERFACE_INSKETCH
#define	QC_INTERFACE			0x00	//0x00 None

#define USARTDRIVER_INSKETCH
#define USART_TXENABLE			0
#define USART_TXENPIN			3

#define USART_DEBUG  			0

#define USARTDRIVER_INSKETCH
#define	USARTDRIVER				Serial3 //Dico al driver vNet di usare la seriale 3 della IBoard Pro

#include "Souliss.h"
#include "SpeakEasy.h"
#include "Typicals.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// network addresses
#define myvNet_address		0xCE03
#define myvNet_subnet		0xFF00
#define myvNet_supern		0x0000

#define TAMB		0   //T19 ID Logico del Led del Tipico 19

#define DEADBAND      0.05  //Se la variazione è superio del 5% aggiorno
#define DEADBANDNTC   0.01  //Se la variazione è superio del 1% aggiorno
#define NODEADBAND	  0 //Se la variazione è superio del 0,1% aggiorno

//Sonda
#define ONE_WIRE_BUS 37
#define TEMPERATURE_PRECISION 9
float t_amb;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;


// flag 
U8 data_chg = 1;

void setup()
{	
	Souliss_SetAddress(myvNet_address, myvNet_subnet, myvNet_supern);		
	Serial.begin(115200);
	Serial.println("INIT");
	//T52 Temperatur DHT
	Souliss_SetT52(memory_map, TAMB);

	sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
	sensors.getAddress(insideThermometer, 0);
}

void loop()
{
	EXECUTEFAST() {						
		UPDATEFAST();
		FAST_50ms() {	// We process the logic and relevant input and output every 50 milliseconds

			
		}

		FAST_70ms() {

		}

		FAST_90ms() { 
			
		}

		FAST_110ms() {
			// Retreive data from the communication channel
			Souliss_CommunicationData(memory_map, &data_chg);
		}

		FAST_510ms() {	// We retrieve data from the node with index 1 (peervNet_address)

		}

		FAST_1110ms() {
			Souliss_Logic_T52(memory_map, TAMB, NODEADBAND, &data_changed);
		}

        FAST_2110ms() {
			DSRead();
        }

}
	
	EXECUTESLOW() {
		UPDATESLOW();

		SLOW_10s() {		// We handle the light timer with a 10 seconds base time
			
		}
	}		
}

void DSRead() {
	//Leggo la sonda DHT
	sensors.requestTemperatures();
	float t_amb = sensors.getTempC(insideThermometer);
	Souliss_ImportAnalog(memory_map, TAMB, &t_amb);

	 Serial.print("Temp C: ");
	 Serial.println(t_amb);
}
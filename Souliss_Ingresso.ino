/**************************************************************************
	--- Souliss Friariello ---

        Controllo Ingresso
			- Temperatura
			- Conteggio Kw/h
			- Controllo Luci ingresso

***************************************************************************/

#define USARTDRIVER_INSKETCH
#define USART_TXENABLE			0
#define USART_TXENPIN			3

#define USART_DEBUG  			0

// Configure the framework
#include "bconf/StandardArduino.h"				// Use a standard Arduino
#include "conf/ethENC28J60.h"					// Ethernet through ENC28j60

// Include framework code and libraries
#include <SPI.h>
#include "Souliss.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Define the network configuration according to your router settings
uint8_t ip_address[4]  = {192, 168, 1, 130};
uint8_t subnet_mask[4] = {255, 255, 255, 0};
uint8_t ip_gateway[4]  = {192, 168, 1, 1};

// network addresses
#define eth_address				ip_address[3]	// The last byte of the IP address (130) is also the vNet address
#define myvNet_subnet			0xFF00
#define myvNet_supern			0x0000

#define TAMB		0			//Slot che contiene la temperatura ambiente

#define DEADBAND      0.05  //Se la variazione è superio del 5% aggiorno
#define DEADBANDNTC   0.01  //Se la variazione è superio del 1% aggiorno
#define NODEADBAND	  0 //Se la variazione è superio del 0,1% aggiorno

//Sonda
#define ONE_WIRE_BUS			37
#define TEMPERATURE_PRECISION	9
float t_amb;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;


// flag 
U8 data_chg = 1;

void setup()
{	
	Souliss_SetAddress(eth_address, myvNet_subnet, myvNet_supern);		
	Serial.begin(115200);
	Serial.println("INIT");

	Souliss_SetT52(memory_map, TAMB);	//Impsoto il tipico per contenere la temperatura

	//Imposto la sonda DS18B20
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
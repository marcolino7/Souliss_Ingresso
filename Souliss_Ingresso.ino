/**************************************************************************
	--- Souliss Friariello ---

        Controllo Ingresso
			- Temperatura
			- Conteggio Kw/h
			- Controllo Luci ingresso
			- Verifica se luci ingresso sono accese

***************************************************************************/

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

#define TAMB			0			//Slot che contiene la temperatura ambiente
#define WATT			2			//Slot per contenere i kW/h letti
#define LIGHT_SW		4			//Comando accensione spegnimento luce corridoio
#define LIGHT_STATUS	5			//Slot che contiene lo stato delle luci all'ingresso

#define DEADBAND      0.05			//Se la variazione è superio del 5% aggiorno
#define DEADBANDNTC   0.01			//Se la variazione è superio del 1% aggiorno
#define NODEADBAND	  0				//Se la variazione è superio del 0,1% aggiorno

#define PIN_LIGHT_SW		5		//Piedino che controlla il relè
#define PIN_LIGHT_STATUS	6		//Piedino che verifica lo stato delle luci
#define PIN_DS18B20			37		//Piedino della sonda DS18B20

//Sonda Temperatura
#define TEMPERATURE_PRECISION	9	
float t_amb;
OneWire oneWire(PIN_DS18B20);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer;

//Conteggio dei watt
long pulseCount = 0;				//Number of pulses, used to measure energy.
unsigned long pulseTime,lastTime;	//Used to measure power.
double power, elapsedkWh;			//power and energy
//Number of pulses per wh - found or set on the meter.
int ppwh = 1; //1000 pulses/kwh = 1 pulse per wh

// flag 
U8 data_chg = 1;

void setup()
{	
	Souliss_SetAddress(eth_address, myvNet_subnet, myvNet_supern);		
	Serial.begin(115200);
	Serial.println("Node INIT");

	
	Souliss_SetT52(memory_map, TAMB);	//Impsoto il tipico per contenere la temperatura
	Souliss_SetT57(memory_map, WATT);	//Imposto il tipico per contenere i watt	
	Souliss_SetT14(memory_map, LIGHT_SW);	//Impsoto il tipico per contenere la temperatura

	//Imposto la sonda DS18B20
	sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
	sensors.getAddress(insideThermometer, 0);

	//Imposto l'iterrupt IRQ1 al pin 3 sul fronte di salita
	attachInterrupt(1, onPulse, FALLING);
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

		FAST_510ms() {	
			//Logica per controllare il relè delle luci
			Souliss_Logic_T14(memory_map,LIGHT_SW,&data_changed);
			Souliss_DigOut(PIN_LIGHT_SW,Souliss_T1n_OnCoil,memory_map,LIGHT_SW);

			//Logica per controllare il Watt Meter
			Souliss_Logic_T57(memory_map, WATT, NODEADBAND, &data_changed);
		}

		FAST_1110ms() {
			//Logica per controllare la sonda di temperatura
			Souliss_Logic_T52(memory_map, TAMB, NODEADBAND, &data_changed);
		}

        FAST_2110ms() {
			DSRead();	//Routine per leggere il valore della sonda e importarlo in Souliss
        }

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
	Souliss_ImportAnalog(memory_map, TAMB, &t_amb);

	 Serial.print("Temp C: ");
	 Serial.println(t_amb);
}

void onPulse() {
	// The interrupt routine, viene lanciata a ogni lampeggio del led

	//usato per misurare il tempo tra gli impulsi.
	lastTime = pulseTime;
	pulseTime = micros();

	//pulseCounter
	pulseCount++;

	//Calcolo la potenza
	power = (3600000000.0 / (pulseTime - lastTime))/ppwh;
  
	//kw/h consumati
	elapsedkWh = (1.0*pulseCount/(ppwh*1000)); //multiply by 1000 to pulses per wh to kwh convert wh to kwh

	float f_power = (float) power;
	Souliss_ImportAnalog(memory_map, WATT, &f_power);

	//Print the values.
	Serial.print(power,4);
	Serial.print(" ");
	Serial.println(elapsedkWh,3);
}
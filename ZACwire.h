/*	ZACwire - Library for reading temperature sensors TSIC 206/306/506
	created by Adrian Immer in 2020
	v2.0.0
*/

#ifndef ZACwire_h
#define ZACwire_h
#include <stddef.h>
#include <stdint.h>
#include "driver/gpio.h"
#include "esp_system.h"

class ZACwire {
	public:
		static const int errorNotConnected	{-400};
		static const int errorMisreading	{-401};
		explicit ZACwire(gpio_num_t inputPin, int16_t sensor=716);
		bool begin();	 //start reading
		float getTemp();	//return temperature in °C
		void end();

		
	private:
		const int timeout		{2000};	//timeout in ms to give error 221
		static void IRAM_ATTR isrHandler(void* ptr);
		void IRAM_ATTR read();			//ISR to record every rising edge
		gpio_num_t _pin;
		int16_t _sensor;
		int64_t lastFallingEdge;			//timestamp of last falling edge
		int64_t measuredTime;			//measured time between falling edges of signal
		volatile int pulseCounter;
		volatile int value, temp;
		bool parity;
};


#endif

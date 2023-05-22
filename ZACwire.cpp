/*	ZACwire - Library for reading temperature sensors TSIC 206/306/506
	created by Adrian Immer in 2020 -  Version 2.0.0

	ESP-IDF port by Walter Jerusalinsky in 2023  -  Version 2.1.0
	Added support for 14 bit sensors (TSIC 716) 
	v2.1.0
*/

#include "ZACwire.h"
#include "esp_timer.h"

ZACwire::ZACwire(gpio_num_t pin, int16_t sensor) : _pin{pin}, _sensor{sensor}
{	queue=xQueueCreate(1, sizeof(int));
	pulseCounter = -2;
}

bool ZACwire::begin()
{ // start collecting data, needs to be called over 2ms before first getTemp()
	gpio_reset_pin(_pin);
	gpio_set_direction(_pin, GPIO_MODE_INPUT);
	gpio_set_pull_mode(_pin, GPIO_PULLUP_ONLY);
	gpio_install_isr_service(ESP_INTR_FLAG_LOWMED);
	gpio_set_intr_type(_pin, GPIO_INTR_ANYEDGE);
	gpio_isr_handler_add(_pin, isrHandler, this);
	return true;
}

float ZACwire::getTemp()
{	int64_t time = esp_timer_get_time();
	
	/*while(pulseCounter!=-1){ // wait for end of reading
		if(pulseCounter==-3) // error
			return errorMisreading;
		if(esp_timer_get_time()-time > timeout*1000)
			return errorNotConnected;
	}*/
	// return temperature in °C
	///gpio_intr_disable(_pin);
	//int temp = this->temp;
	//gpio_intr_enable(_pin); */
	int temp;
	if(!xQueueReceive(queue, &temp, pdMS_TO_TICKS(timeout))){
		return errorNotConnected;
	};
	if(!temp)
		return errorMisreading;
	if (_sensor < 316) // LT=-50 HT=150 11 bits
		return ((temp * 200.0 / 2047) - 50.0);
	else if(_sensor==316) // LT=-50°C HT=150°C 14 bits
		return ((temp * 200.0 / 16383) - 50.0);
	else if(_sensor < 516) // LT=-10°C HT=60°C 11 bits
		return ((temp * 70.0 / 2047) - 10.0);
	else if(_sensor==516) // LT=-10°C HT=60°C 14 bits
		return ((temp * 70.0 / 16383) - 10.0);
	else	// 716
		return ((temp * 70.0 / 16383) - 10.0);
}

void ZACwire::end()
{
	gpio_isr_handler_remove(_pin);
	gpio_uninstall_isr_service();
}

void ZACwire::isrHandler(void *ptr)
{
	static_cast<ZACwire *>(ptr)->read();
}

void ZACwire::read()
{ 	// get called with every rising and falling edge
	int64_t irqTime = esp_timer_get_time();
	if( gpio_get_level(_pin) )
	{ // rising edge (data)
		if(measuredTime){
			int micros = irqTime - lastFallingEdge;
			int data = micros < strobeTime ? 1 : 0;
			switch(pulseCounter){
				case -3: // error
				case -2: // first start
				case -1: // reading done;
					break;
				case 0:
				case 10: // start bit
					strobeTime = micros;
					break;
				case 9: // parity
				case 19: //parity
					if(parity!=data)
					{
						pulseCounter=-3;
						break;
					}
					parity=0;
					if(pulseCounter==19) // last bit received
					{
						//temp=value;
						xQueueOverwriteFromISR(queue, (const void *) &value, NULL);
						pulseCounter=-1;
					}
					break;
				default:
					value=(value<<1)|data;
					parity^=data;
			}
		}
	}
	else
	{ 	// falling edge (clk)
		if(lastFallingEdge) {
			measuredTime = irqTime - lastFallingEdge; // measure time to previous rising edge (clk)
			if(measuredTime > 1000) { // First start
				pulseCounter = 0;
				value = 0;
				parity = 0;
			}
			else
			{
				pulseCounter++;
			}
		}
		lastFallingEdge = irqTime;
	}
}



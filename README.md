# ESP-IDF port of ZACwire™ Library to read TSic sensors
[![GitHub issues](https://img.shields.io/github/issues/walterjj/esp-idf-zacwire.svg)](https://github.com/walterjj/esp-idf-zacwire/issues/) 
[![GitHub license](https://img.shields.io/github/license/walterjj/esp-idf-zacwire.svg)](https://github.com/walterjj/esp-idf-zacwire/blob/master/LICENSE)


ESP-IDF Component to read the ZACwire protocol, wich is used by TSic temperature sensors 206, 306, 506, 716 on their signal pin.

`ZACwire obj(int pin, int sensor)` tells the library which input pin of the controller (eg. 2) and type of sensor (eg. 306) it should use. Please pay attention that the selected pin supports external interrupts!

`.begin()` returns true if a signal is detected on the specific pin and starts the reading via ISRs. It should be started at least 2ms before the first .getTemp().

`.getTemp()` returns the temperature in °C and gets usually updated every 100ms. In case of a failed reading, it returns `222`. In case of no incoming signal it returns `221`.

`.end()` stops the reading for time sensititive tasks, which shouldn't be interrupted.


## Benefits compared to former TSic libraries
- saves a lot of controller time, because no delay() is used and calculations are done by bit manipulation
- low memory consumption
- misreading rate lower than 0.001%
- reading an unlimited number of TSic simultaneously (except AVR boards)
- higher accuracy (0.1°C offset corrected)
- simple use






## Example
```c++
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ZACwire.h"

ZACwire zacWire((gpio_num_t) 21, 716);		// set pin "21" to receive signal from the TSic "716"


void temperature_task(void *pvParameters){
    float temp;
    zacWire.begin();
    while (true)
    {   temp=zacWire.getTemp();
        if(temp==ZACwire::errorMisreading) {
            printf("Error reading temperature\n");
        }
        else if(temp==ZACwire::errorNotConnected) {
            printf("No sensor connected\n");
        }
        else {
          printf("Temperature: %.02f deg.C\n", temp);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

extern "C" void app() {
        xTaskCreate(temperature_task, "temperature_task", configMINIMAL_STACK_SIZE * 8, NULL, 5, NULL);
        printf("APP START\n");

```



## Wiring
Connect V+ to a power supply with 3.0V to 5.5V. For most accurate results connect it to 5V, because that's the voltage the sensor was calibrated with.

The output of the signal pin switches between GND and V+ to send informations, so take care that your µC is capable of reading both V+ and GND.

![TSIC](https://user-images.githubusercontent.com/62163284/116116897-f5ed5900-a6bb-11eb-95b8-ba8f4ef129cc.png)


## Original Library
The original Adrian Immer's Arduino Library can be found [here](https://github.com/lebuni/ZACwire-Library)




/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HT_SENSORS_H
#define __HT_SENSORS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "rf_driver_hal_i2c_ex.h"

/* Private function prototypes -----------------------------------------------*/

void HTS221_Init(void);
void LSP22HB_Init(void);
void initTimer(void);

float readTemperatureHTS221(void);
float readHumidity(void);
float readPressure(void);

void calibrateTemperatureHTS221(void);
void calibrateHumidity(void);

void initSensors(void);

void txLora(void);

typedef struct {
	 int16_t temperature;
	 uint16_t humi;
	 uint8_t p;
} __attribute__((packed)) HT_payload;

/* Private defines -----------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __HT_SENSORS_H */


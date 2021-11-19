#include "i2c.h"
#include "ht_sensors.h"
#include "main.h"
#include "lorawan_setup.h"

#define APP_SENSORS_DUTYCYCLE   15000		/* Timer timeout in milliseconds */

static void OnSensorsTimerEvent( void* context );  /* Timer callback function */

static TimerEvent_t sensorsTimer;

uint8_t dados [2];
uint16_t devAdd_read = 0xBF; 		/* HTS221 i2c read address */
uint16_t devAdd_write = 0xBE;		/* HTS221 i2c write address */	
uint16_t T0_degC, T1_degC;
int16_t T0_OUT, T1_OUT, T_OUT;
float temp = 0.0;
float P = 0.0;

uint16_t H0_T0_OUT, H1_T0_OUT, H_T_OUT;
float humidity = 0.0;
uint16_t H0_rH_x2, H1_rH_x2, H0_rH, H1_rH;

HT_payload buff = {0};

void HTS221_Init(void){
	
	uint8_t reg = 0x82;	
	HAL_I2C_Mem_Write(&hi2c, 0xBE, 0x20, I2C_MEMADD_SIZE_8BIT, &reg, 1, 500);
	
	calibrateTemperatureHTS221(); /* Gets HTS221 sensor calibration data */	
}

void LSP22HB_Init(void){
	uint8_t dados[2];
	dados[0] = 0X3A;    
	dados[1] = 0X10;
	
	HAL_I2C_Mem_Write(&hi2c, 0xBA, 0x10, I2C_MEMADD_SIZE_8BIT,&dados[0],1,500); //REG1
	HAL_I2C_Mem_Write(&hi2c, 0XBA, 0x11, I2C_MEMADD_SIZE_8BIT,&dados[1],1,500); //REG2
	
	calibrateHumidity();	/* Gets LSP22HB sensor calibration data */
}

void calibrateTemperatureHTS221(void){

	uint8_t T0_degC_x8_u8, T1_degC_x8_u8, x;
	dados[0] = 0;
	
	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x32, I2C_MEMADD_SIZE_8BIT, &dados[0], 1,500);
	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x33, I2C_MEMADD_SIZE_8BIT, &dados[1], 1,500);
	T0_degC_x8_u8 = dados[0] & 0xFF;
	T1_degC_x8_u8 = dados[1] & 0xFF;
	
	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x35, I2C_MEMADD_SIZE_8BIT, &dados[0], 1,500);
	x = dados[0] &0x0F;
	T0_degC = ((x & 0x03) * 256) + T0_degC_x8_u8;
	T1_degC = ((x & 0x0C) * 64) + T1_degC_x8_u8;
	
	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x3C, I2C_MEMADD_SIZE_8BIT, &dados[0], 1,500);
	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x3D, I2C_MEMADD_SIZE_8BIT, &dados[1], 1,500);
	T0_OUT = ((dados[1] & 0xFF) * 256) + (dados[0] & 0xFF);

	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x3E, I2C_MEMADD_SIZE_8BIT, &dados[0], 1,500);
	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x3F, I2C_MEMADD_SIZE_8BIT, &dados[1], 1,500);
	T1_OUT = ((dados[1] & 0xFF) * 256) + (dados[0] & 0xFF);
}

void calibrateHumidity(void){
	
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x30, I2C_MEMADD_SIZE_8BIT, &dados[0], 1 , 500);
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x31, I2C_MEMADD_SIZE_8BIT, &dados[1], 1 , 500);
	H0_rH_x2 = dados[0];
	H1_rH_x2 = dados[1];
	
	H0_rH = H0_rH_x2 >> 1;  
	H1_rH = H1_rH_x2 >> 1;  
	
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x36, I2C_MEMADD_SIZE_8BIT, &dados[0], 1 , 500);
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x37, I2C_MEMADD_SIZE_8BIT, &dados[1], 1 , 500);
	
	H0_T0_OUT = ((dados[1] & 0xFF) * 256) + (dados[0] & 0xFF); 
	
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x3A, I2C_MEMADD_SIZE_8BIT, &dados[0], 1 , 500);
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x3B, I2C_MEMADD_SIZE_8BIT, &dados[1], 1 , 500);
	
	H1_T0_OUT = ((dados[1] & 0xFF) * 256) + (dados[0] & 0xFF); 
	
}
	

void initSensors(void){
	
			HTS221_Init();			/* Initializes HTS221 temperature and humidity sensor */
			LSP22HB_Init();			/* Initializes LSP22HB pressure sensor */
			initTimer();				/* Inirialize application timer */
}

void initTimer(void){
			
			TimerInit( &sensorsTimer, OnSensorsTimerEvent ); /* Timer initialization */
      TimerSetValue( &sensorsTimer, APP_SENSORS_DUTYCYCLE);	/* Sets alarm total time */			
			TimerStart(&sensorsTimer);	/* Callback call */
}

static void OnSensorsTimerEvent( void* context ){
	
			HAL_I2C_MspInit(&hi2c);
			readTemperatureHTS221();		/* Reads HTS221 temperature data */
		  readHumidity();							/* Reads HTS221 humidity data */
			readPressure();							/* Reads LSP22HB pressure data */
			HAL_I2C_MspDeInit(&hi2c);
			TimerStart(&sensorsTimer);
			txLora();										/* Sends Lora packet to the gateway */			
  }

float readTemperatureHTS221(void){

	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x2A, I2C_MEMADD_SIZE_8BIT, &dados[0], 1,500);
	HAL_I2C_Mem_Read(&hi2c, devAdd_read, 0x2B, I2C_MEMADD_SIZE_8BIT, &dados[1], 1,500);
	T_OUT = ((dados[1]*256) + dados[0]);
	
	temp = ((T1_degC - T0_degC) / 8.0) * (T_OUT - T0_OUT) / (T1_OUT - T0_OUT) + (T0_degC / 8.0);
	
	printf("\nTemperature: %.4f ºC\n", temp);
	return temp;
}

float readHumidity(void) {	
	printf("Sampling humidity\n");
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x28, I2C_MEMADD_SIZE_8BIT, &dados[0], 1 , 500);
	HAL_I2C_Mem_Read(&hi2c, 0xBF, 0x29, I2C_MEMADD_SIZE_8BIT, &dados[1], 1 , 500);
	
	H_T_OUT = ((dados[1] & 0xFF) * 256) + (dados[0] & 0xFF);;//(dados[1] << 8) | dados[0];
	
	humidity = ((float)((H1_rH - H0_rH) * (H_T_OUT - H0_T0_OUT))/(float)(H1_T0_OUT - H0_T0_OUT)) + H0_rH;
	
	printf("Humidity: %f\n", humidity);
		
	return humidity;	
}

float readPressure(void){
	uint8_t dados[3];
	uint8_t send[30];
	int32_t aux;
	
	printf("Sampling pressure\n");
	
	HAL_I2C_Mem_Read(&hi2c, 0xBB, 0x28, I2C_MEMADD_SIZE_8BIT, &dados[0], 1,500);
	HAL_I2C_Mem_Read(&hi2c, 0xBB, 0x29, I2C_MEMADD_SIZE_8BIT, &dados[1], 1,500);
	HAL_I2C_Mem_Read(&hi2c, 0xBB, 0x2A, I2C_MEMADD_SIZE_8BIT, &dados[2], 1,500);
	
	aux = (((uint32_t)dados[2]) << 16) | (((uint32_t)dados[1]) << 8) | ((uint32_t)dados[0]);
	P = ((float)aux)/4095; /* Return pressure in hPa */
	P = P * 0.0009869233;  /* Convert pressure to atm */
	
	return P;
}

void txLora(void){
		HT_payload buff;
		lora_AppData_t appData;
		
	  buff.temperature = temp*100;
		buff.humi = humidity*100;
		buff.p = P*100;
	
		printf("temperature: %.2f  ---- %04X\n", temp, buff.temperature); 
		printf("humidity: %.2f  ---- %04X\n",humidity, buff.humi); 
		printf("pressure: %.2f  ---- %04X\n",P, buff.p); 
	
	  printf("\nBuffer TX: 0x%04X%04X%02X", buff.temperature, buff.humi, buff.p);
	
		appData.Buff = (uint8_t *)&buff;

		appData.BuffSize = sizeof(HT_payload);
		appData.Port = LORAWAN_APP_PORT;
	
		lorawan_send(&appData);
}

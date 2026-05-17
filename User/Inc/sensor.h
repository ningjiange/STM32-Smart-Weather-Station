#ifndef __SENSOR_H
#define __SENSOR_H

#include <stdint.h>

extern uint8_t dht11_debug_code;

void LightSensor_Read(void);
void DHT11_TIM_Init(void);
void DHT11_Read(void);
void MPU6050_Init(void);
void MPU6050_Read(void);
#endif

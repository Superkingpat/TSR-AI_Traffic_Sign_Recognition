#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

#include "stm32f4xx_hal.h"

// Function prototypes
void accel_init(I2C_HandleTypeDef hi2c1);
void accel_read(I2C_HandleTypeDef hi2c1, int16_t data[3]);

#endif // ACCELEROMETER_H

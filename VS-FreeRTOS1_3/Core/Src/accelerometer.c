#include "accelerometer.h"

void accel_init(I2C_HandleTypeDef hi2c1) {
	uint8_t par1 = 0x27, par2 = 0x88;
	HAL_I2C_Mem_Write(&hi2c1, 0x19<<1, 0x20, I2C_MEMADD_SIZE_8BIT, &par1, 1, 10);
	HAL_I2C_Mem_Write(&hi2c1, 0x19<<1, 0x23, I2C_MEMADD_SIZE_8BIT, &par2, 1, 10);
}

void accel_read(I2C_HandleTypeDef hi2c1, int16_t data[3]) {
	HAL_I2C_Mem_Read(&hi2c1, 0x32, 0xA8, I2C_MEMADD_SIZE_8BIT, (uint8_t*)&data, 6, 6);
}

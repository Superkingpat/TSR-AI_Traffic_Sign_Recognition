#ifndef LCD_H
#define LCD_H

#include "stm32f4xx_hal.h"

// LCD I2C slave address
#define SLAVE_ADDRESS_LCD 0x4E

// Function prototypes
void lcd_init(I2C_HandleTypeDef hi2c1);
void lcd_send_cmd(I2C_HandleTypeDef hi2c1, char cmd);
void lcd_send_data(I2C_HandleTypeDef hi2c1, char data);
void lcd_send_string(I2C_HandleTypeDef hi2c1, char *str);
void lcd_put_cur(I2C_HandleTypeDef hi2c1, int row, int col);

#endif // LCD_H

#include "lcd.h"
//FROM https://controllerstech.com/i2c-lcd-in-stm32/
// Send command to the LCD
void lcd_send_cmd(I2C_HandleTypeDef hi2c1, char cmd) {
    char data_u = cmd & 0xF0;
    char data_l = (cmd << 4) & 0xF0;
    uint8_t data_t[4];

    data_t[0] = data_u | 0x0C;  // en=1, rs=0
    data_t[1] = data_u | 0x08;  // en=0, rs=0
    data_t[2] = data_l | 0x0C;  // en=1, rs=0
    data_t[3] = data_l | 0x08;  // en=0, rs=0

    HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, data_t, 4, 100);
}

// Send data to the LCD
void lcd_send_data(I2C_HandleTypeDef hi2c1, char data) {
    char data_u = data & 0xF0;
    char data_l = (data << 4) & 0xF0;
    uint8_t data_t[4];

    data_t[0] = data_u | 0x0D;  // en=1, rs=1
    data_t[1] = data_u | 0x09;  // en=0, rs=1
    data_t[2] = data_l | 0x0D;  // en=1, rs=1
    data_t[3] = data_l | 0x09;  // en=0, rs=1

    HAL_I2C_Master_Transmit(&hi2c1, SLAVE_ADDRESS_LCD, data_t, 4, 100);
}

// Initialize the LCD
void lcd_init(I2C_HandleTypeDef hi2c1) {
    HAL_Delay(50);  // Wait for >40ms
    lcd_send_cmd(hi2c1, 0x30);
    HAL_Delay(5);   // Wait for >4.1ms
    lcd_send_cmd(hi2c1, 0x30);
    HAL_Delay(1);   // Wait for >100us
    lcd_send_cmd(hi2c1, 0x30);
    HAL_Delay(10);
    lcd_send_cmd(hi2c1, 0x20);  // 4-bit mode

    // Display initialization
    lcd_send_cmd(hi2c1, 0x28); // Function set: 4-bit mode, 2 lines, 5x8 font
    HAL_Delay(1);
    lcd_send_cmd(hi2c1, 0x08); // Display off
    HAL_Delay(1);
    lcd_send_cmd(hi2c1, 0x01); // Clear display
    HAL_Delay(2);
    lcd_send_cmd(hi2c1, 0x06); // Entry mode set: increment cursor
    HAL_Delay(1);
    lcd_send_cmd(hi2c1, 0x0C); // Display on, cursor off, blink off
}

// Send a string to the LCD
void lcd_send_string(I2C_HandleTypeDef hi2c1, char *str) {
    while (*str) {
        lcd_send_data(hi2c1, *str++);
    }
}

// Set cursor position on the LCD
void lcd_put_cur(I2C_HandleTypeDef hi2c1, int row, int col) {
    switch (row) {
        case 0: col |= 0x80; break;
        case 1: col |= 0xC0; break;
    }
    lcd_send_cmd(hi2c1, col);
}

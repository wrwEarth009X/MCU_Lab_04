/*
 * i2c-lcd.h
 *
 *  Created on: Sep 11, 2026
 *      Author: Sam@CPE_RMUTT_uC-Lab
 */

#ifndef I2C_LCD_H_
#define I2C_LCD_H_

#include "stm32f1xx_hal.h"

void lcd_init(void);   // initialize lcd
void lcd_send_cmd(char cmd);  // send command to lcd
void lcd_send_data(char data);  // send data to lcd
void lcd_send_string(char *str);  // send string to lcd
void lcd_put_cur(int row, int col);  // put cursor at the entered position
void lcd_clear(void);
void lcd_backlight_on(void);
void lcd_backlight_off(void);

// ฟังก์ชันสำหรับ Scrolling Text ตามโจทย์
void lcd_scroll_text(int row, char *str, int delay_ms);

#endif /* I2C_LCD_H_ */

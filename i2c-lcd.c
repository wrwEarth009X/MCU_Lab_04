#include "i2c-lcd.h"

extern I2C_HandleTypeDef hi2c1;  // ดึงตัวแปร I2C จาก main.c มาใช้

#define SLAVE_ADDRESS_LCD 0x4E // เปลี่ยนเป็น 0x7E หากจอของคุณใช้ Address 0x3F
static uint8_t backlight_state = 0x08;	// เปิดใช้งานตัวแปรควบคุม Backlight (0x08 = เปิด, 0x00 = ปิด)

//-- ฟังก์ชันส่งคำสั่ง (แบบรองรับ Backlight) --
void lcd_send_cmd (char cmd) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd&0xf0);
    data_l = ((cmd<<4)&0xf0);

    data_t[0] = data_u | 0x04 | backlight_state;  // en=1, rs=0
    data_t[1] = data_u | backlight_state;         // en=0, rs=0
    data_t[2] = data_l | 0x04 | backlight_state;  // en=1, rs=0
    data_t[3] = data_l | backlight_state;         // en=0, rs=0
    HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);
}

//-- ฟังก์ชันส่งข้อมูล (แบบรองรับ Backlight) --
void lcd_send_data (char data) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (data&0xf0);
    data_l = ((data<<4)&0xf0);

    data_t[0] = data_u | 0x05 | backlight_state;  // en=1, rs=1
    data_t[1] = data_u | backlight_state;         // en=0, rs=1
    data_t[2] = data_l | 0x05 | backlight_state;  // en=1, rs=1
    data_t[3] = data_l | backlight_state;         // en=0, rs=1
    HAL_I2C_Master_Transmit (&hi2c1, SLAVE_ADDRESS_LCD, (uint8_t *)data_t, 4, 100);
}

void lcd_clear (void) {
    lcd_send_cmd (0x01);
    HAL_Delay(2);
}

void lcd_put_cur(int row, int col) {
    switch (row) {
        case 0: col |= 0x80; break;
        case 1: col |= 0xC0; break;
    }
    lcd_send_cmd (col);
}

void lcd_init (void) {
    HAL_Delay(50);
    lcd_send_cmd (0x30);
    HAL_Delay(5);
    lcd_send_cmd (0x30);
    HAL_Delay(1);
    lcd_send_cmd (0x30);
    HAL_Delay(10);
    lcd_send_cmd (0x20);  // โหมด 4-bit
    HAL_Delay(10);

    lcd_send_cmd (0x28);  // ตั้งค่า 2 แถว, ฟอนต์ 5x8
    HAL_Delay(1);
    lcd_send_cmd (0x0C);  // เปิดจอ, ปิดเคอร์เซอร์
    HAL_Delay(1);
    lcd_send_cmd (0x06);  // เลื่อนเคอร์เซอร์ไปทางขวาอัตโนมัติ
    HAL_Delay(1);
    lcd_send_cmd (0x01);  // เคลียร์หน้าจอ
    HAL_Delay(2);
}

void lcd_send_string (char *str) {
    while (*str) lcd_send_data (*str++);
}

void lcd_backlight_on(void) {
    backlight_state = 0x08;
    lcd_send_cmd(0x00); // อัปเดตสถานะไฟหลังจอ
}

void lcd_backlight_off(void) {
    backlight_state = 0x00;
    lcd_send_cmd(0x00); // อัปเดตสถานะไฟหลังจอ
}

//-- ฟังก์ชันเลื่อนข้อความวิ่งจากขวาไปซ้าย (Scrolling Text) ตามโจทย์ข้อ 3 --
void lcd_scroll_text(int row, char *str, int delay_ms) {
    int len = 0;
    while(str[len] != '\0') len++; // หาความยาวข้อความ

    char display_str[17];
    for (int i = 0; i < len + 16; i++) {
        // เช็กการกดปุ่ม (PA0 หรือ PA1) ถ้ามีการกดปุ่มให้ออกจากฟังก์ชันวิ่งข้อความทันที
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET ||
            HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET) {
            return;
        }

        for (int j = 0; j < 16; j++) {
            int char_index = i - 15 + j;
            if (char_index >= 0 && char_index < len) {
                display_str[j] = str[char_index];
            } else {
                display_str[j] = ' ';
            }
        }
        display_str[16] = '\0';

        lcd_put_cur(row, 0);
        lcd_send_string(display_str);
        HAL_Delay(delay_ms);
    }
}

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c-lcd.h"
#include <stdio.h>
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TOTAL_MEMBERS 2 // จำนวนสมาชิกในกลุ่ม
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

/* USER CODE BEGIN PV */
char *member_names[TOTAL_MEMBERS] = {
    "1. Weerawoot D.",
    "2. Puripat M."
};

char *member_ids[TOTAL_MEMBERS] = {
    "ID: 116830462023-4",
    "ID: 116830462025-9"
};

uint8_t mode = 0;       // 0 = หน้าแรก, 1 = แสดงสมาชิก
int current_member = 0; // ลำดับสมาชิกปัจจุบัน

// ตัวแปรสำหรับการเลื่อนข้อความแบบ Non-blocking
char scroll_msg[] = "<< Press button to show >>";
int scroll_pos = 0;
uint32_t last_scroll_time = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */
void show_welcome_screen(void);
void show_member_info(int index);
void update_scrolling_text(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void show_welcome_screen(void)
{
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string("Member List");
    scroll_pos = 0;
    last_scroll_time = HAL_GetTick();
}

void show_member_info(int index)
{
    lcd_clear();
    lcd_put_cur(0, 0);
    lcd_send_string(member_names[index]);
    lcd_put_cur(1, 0);
    lcd_send_string(member_ids[index]);
}

// ฟังก์ชันเลื่อนข้อความแบบ Non-blocking ไม่ดักรอ Delay
void update_scrolling_text(void)
{
    if (HAL_GetTick() - last_scroll_time >= 250) // ทำงานทุกๆ 250 ms
    {
        last_scroll_time = HAL_GetTick();

        int len = strlen(scroll_msg);
        char display_str[17];

        for (int j = 0; j < 16; j++) {
            int char_index = scroll_pos - 15 + j;
            if (char_index >= 0 && char_index < len) {
                display_str[j] = scroll_msg[char_index];
            } else {
                display_str[j] = ' ';
            }
        }
        display_str[16] = '\0';

        lcd_put_cur(1, 0);
        lcd_send_string(display_str);

        scroll_pos++;
        if (scroll_pos >= len + 16) {
            scroll_pos = 0;
        }
    }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();

  /* USER CODE BEGIN 2 */
  lcd_init();
  show_welcome_screen();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // --- ตรวจสอบการกดปุ่มที่ 1 (PA0: แสดงรายชื่อสมาชิก) ---
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
    {
        HAL_Delay(200); // Debounce ป้องกันสัญญาณเด้งเบิ้ล
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) // ยืนยันการกดปุ่มจริง
        {
            if (mode == 0)
            {
                mode = 1;
                current_member = 0; // เริ่มแสดงคนแรก
            }
            else
            {
                current_member = (current_member + 1) % TOTAL_MEMBERS; // วนแสดงคนถัดไป
            }
            show_member_info(current_member);

            // รอจนกว่าจะปล่อยปุ่มเพื่อไม่ให้คำสั่งทำงานรวดเดียว
            while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET);
        }
    }

    // --- ตรวจสอบการกดปุ่มที่ 2 (PA1: กลับหน้าแรก) ---
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET)
    {
        HAL_Delay(200); // Debounce ป้องกันสัญญาณเด้งเบิ้ล
        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET) // ยืนยันการกดปุ่มจริง
        {
            mode = 0;
            show_welcome_screen();

            // รอจนกว่าจะปล่อยปุ่ม
            while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET);
        }
    }

    // --- ถ้าอยู่ในโหมดหน้าแรก ให้เลื่อนข้อความวิ่งบรรทัดที่ 2 ---
    if (mode == 0)
    {
        update_scrolling_text();
    }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /* ตั้งค่า PA0 (ปุ่ม 1) และ PA1 (ปุ่ม 2) เป็น Input Pull-Up */
  GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */

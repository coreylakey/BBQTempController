/*
 * Project name:
      THERMO click - Thermocouple Test
 * Copyright:
      (c) mikroElektronika, 2012.
 * Revision History:
      20120709:
      - Initial release (DO);
     20120731:
       - modified for EasyMx PRO v7 for STM32 (FJ);
 * Description:
      This is a simple project which demonstrates the use of THERMO click board.
      Temperature measured by the thermocouple is converter by MAX31855 and displayed on TFT.
      Displayed temperature is in degree Celsius.
      Example also includes fault detection. (Open circuit, Short to GND and Short to VCC).
 * Test configuration:
     MCU:             STM32F107VC
                      http://www.st.com/internet/com/TECHNICAL_RESOURCES/TECHNICAL_LITERATURE/DATASHEET/CD00220364.pdf
     Dev. Board:      EasyMx PRO v7 for STM32
                      http://www.mikroe.com/eng/products/view/852/easymx-pro-v7-for-stm32/
     Oscillator:      HS-PLL 72.0000 MHz, internal 8.0000 MHz RC
     Ext. Modules:    THERMO click - ac:THERMO_click
                      http://www.mikroe.com/eng/products/view/930/thermo-click/
                      EasyTFT Board
                      http://www.mikroe.com/eng/products/view/277/various-components/
     SW:              mikroC PRO for ARM
                      http://www.mikroe.com/mikroc/arm/
 * NOTES:
      - Place THERMO Click board in the mikroBUS socket 1.
      - Place EasyTFT and turn on TFT backlight switch SW11.3.
 */

#include "resources.h"

sbit THERMO_CS at GPIOD_ODR.B13;

int tmp, intTemp, remTemp;
float temperature, temperature_old;
char s_temp[16];
short out_size;
short address, buffer;
unsigned short temp_byte[4] = {0, 0, 0, 0};

void MAX31855_Read() {
  THERMO_CS = 0;
  Delay_ms(350);
  temp_byte[0] = SPI_Read(0);
  temp_byte[1] = SPI_Read(0);
  temp_byte[2] = SPI_Read(0);
  temp_byte[3] = SPI_Read(0);
  THERMO_CS = 1;
}

void Display_Temp_Value(void) {
  out_size = 0;
  tmp = temp_byte[0];
  tmp = tmp << 8;
  tmp = tmp | temp_byte[1];
  remTemp = tmp >> 2;
  remTemp = remTemp & 0x03;                     // Decimal part of temperature value
  temperature = remTemp * 0.25;
  intTemp = tmp >> 4;                           // Integer part of temperature value
  temperature += intTemp;                       // Temperature value
  TFT_Write_Text("Temperature:", 80, 120);

  if (out_size > strlen(s_temp)){
    TFT_Write_Text("                ", 80, 140);
  }
  out_size = strlen(s_temp);
  if (temperature_old != temperature) {
    TFT_Set_Font(&TFT_defaultFont, CL_WHITE, FO_HORIZONTAL);
    TFT_Write_Text(s_temp, 175, 120);                      // Display string on TFT
    sprintf(s_temp,"%4.2f °C",temperature);                // Convert temperature value to string
    TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
    TFT_Write_Text(s_temp, 175, 120);                      // Display string on TFT
  }
  temperature_old = temperature;
}

void DrawFrame(){
  TFT_Init(320,240);
  TFT_BLED = 1;
  TFT_Fill_Screen(CL_WHITE);
  TFT_Set_Pen(CL_Black, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_LIne(20,  46, 300,  46);
  TFT_Set_Font(&HandelGothic_BT21x22_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("THERMO  Click  Board  Demo", 30, 14);
  TFT_Set_Font(&Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("EasyMx PRO v7 for STM32", 19, 223);
  TFT_Set_Font(&Verdana12x13_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);
  TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
}

void main() {
  DrawFrame();
  GPIO_Digital_Output(&GPIOD_ODR, _GPIO_PINMASK_13); // Set PORTD.B13 as digital output
  THERMO_CS = 1;
  temperature = 0;

  TFT_Write_Text("Thermo Test Starting...", 80, 80);
  Delay_ms(500);

  SPI3_Init_Advanced(_SPI_FPCLK_DIV128, _SPI_MASTER  | _SPI_8_BIT |
                     _SPI_CLK_IDLE_LOW | _SPI_FIRST_CLK_EDGE_TRANSITION |
                     _SPI_MSB_FIRST | _SPI_SS_DISABLE | _SPI_SSM_ENABLE | _SPI_SSI_1,
                     &_GPIO_MODULE_SPI3_PC10_11_12);

  TFT_Write_Text("Reading temperature...", 80, 100);
  Delay_ms(500);

  while(1) {
    TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
    TFT_Set_Pen(CL_WHITE, 1);
    TFT_Set_Brush(1, CL_WHITE, 0, LEFT_TO_RIGHT, CL_WHITE, CL_WHITE);
    TFT_Rectangle(80, 150, 210, 170);
    MAX31855_Read();
    if((temp_byte[1] & 0x01) == 0x01){          // Fault detection
      TFT_Set_Font(&TFT_defaultFont, CL_RED, FO_HORIZONTAL);
      TFT_Write_Text("Error - ", 80, 150);
      
      if((temp_byte[3] & 0x01) == 0x01){        // Open circuit fault?
        TFT_Write_Text("Open circuit!", 125, 150);
        Delay_ms(500);
      }

      if((temp_byte[3] & 0x02) == 0x02){        // Short to GND fault?
        TFT_Write_Text("Short to GND!", 125, 150);
        Delay_ms(500);
      }

      if((temp_byte[3] & 0x04) == 0x04){        // Short to Vcc fault?
        TFT_Write_Text("Short to VCC!", 125, 150);
        Delay_ms(500);
      }
    }
    else {
      Display_Temp_Value();
    }
    Delay_ms(500);
  }
}
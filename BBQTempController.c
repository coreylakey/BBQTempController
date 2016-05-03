/*
 * Project name:
     Virtual COM Port Demo
 * Description
     Example showing usage USB CDC device that functions as a generic virtual
     COM port. The example echoes data sent via USART terminal.
 * Test configuration:
     MCU:             STM32F107VC
     Dev.Board:       EasyMx_PRO_v7_for_STM32_ARM
                      http://www.mikroe.com/easymx-pro/stm32/
     Oscillator:      72000000 Hz
     SW:              mikroC PRO for ARM
                      http://www.mikroe.com/mikroc/arm/
     Notes:           Example uses CDC Driver included in Windows. An .inf file
                      is supplied with the example in order to match the driver
                      with this device (user must guide driver installation
                      wizard to the VCPDriver folder).

                      Turn on USB VBUS switch.
 */

#include <stdint.h>

void USBDev_CDCParamsChanged( void );
void USBDev_CDCDataReceived(uint16_t size);
void system_setup( void );
void read_temperature( void );
void display_temperature( void );

//Pins
sbit THERMO_CS at GPIOE_ODR.B8;

// Buffer of 64 bytes
char buffer[64];
//Thermo Declarations
int tmp, intTemp, remTemp;
float temperature, temperature_old;
char s_temp[16];
short out_size;
short address;
unsigned short temp_byte[4] = {0, 0, 0, 0};

void main() {
     char my_buffer[20] = "Temperature: ";
     char new_line[3] = "\r\n";
     uint8_t my_size = 12;
     uint8_t new_line_size = 2;

     system_setup();
  // Infinite loop
  while(1){

           USBDev_CDCSendData( my_buffer, my_size );
           read_temperature();
           display_temperature();
           USBDev_CDCSendData( new_line, new_line_size );
           Delay_ms(1000);
           
  }

}

void system_setup( void )
{


  // Initialize CDC Class
  USBDev_CDCInit();

  // Initialize USB device module
  USBDev_Init();

  // Enable USB device interrupt
  NVIC_IntEnable(IVT_INT_OTG_FS);

  // Wait until device is configured (enumeration is successfully finished)
  while(USBDev_GetDeviceState() != _USB_DEV_STATE_CONFIGURED)
    ;


  // Set receive buffer where received data is stored
  USBDev_CDCSetReceiveBuffer(buffer);
  
  GPIO_Digital_Output(&GPIOE_ODR, _GPIO_PINMASK_8); // Set PORTD.B13 as digital output
  THERMO_CS = 1;
  temperature = 0;
  SPI3_Init_Advanced(_SPI_FPCLK_DIV128, _SPI_MASTER  | _SPI_8_BIT |
                     _SPI_CLK_IDLE_LOW | _SPI_FIRST_CLK_EDGE_TRANSITION |
                     _SPI_MSB_FIRST | _SPI_SS_DISABLE | _SPI_SSM_ENABLE | _SPI_SSI_1,
                     &_GPIO_MODULE_SPI3_PC10_11_12);


}

void read_temperature( void )
{
  THERMO_CS = 0;
  Delay_ms(350);
  temp_byte[0] = SPI_Read(0);
  temp_byte[1] = SPI_Read(0);
  temp_byte[2] = SPI_Read(0);
  temp_byte[3] = SPI_Read(0);
  THERMO_CS = 1;

}

void display_temperature( void )
{
     char my_buffer[20] = { 0 };
     int my_size = 12;
     
      out_size = 0;
      tmp = temp_byte[0];
      tmp = tmp << 8;
      tmp = tmp | temp_byte[1];
      remTemp = tmp >> 2;
      remTemp = remTemp & 0x03;                     // Decimal part of temperature value
      temperature = remTemp * 0.25;
      intTemp = tmp >> 4;                           // Integer part of temperature value
      temperature += intTemp;                       // Temperature value
      temperature = temperature * 1.8 + 32;         //Fahrenheit Conversion

      FloatToStr( temperature, my_buffer );
      USBDev_CDCSendData(my_buffer, my_size);

      temperature_old = temperature;

}


// USB interrupt service routine
void USB0Interrupt() iv IVT_INT_OTG_FS{

  // Call library interrupt handler routine
  USBDev_IntHandler();
}

void USBDev_CDCParamsChanged()
{

}

void USBDev_CDCDataReceived(uint16_t size){
  // Prepare receive buffer
  USBDev_CDCSetReceiveBuffer(buffer);
  // Send back received packet
  //USBDev_CDCSendData(buffer, size);
}
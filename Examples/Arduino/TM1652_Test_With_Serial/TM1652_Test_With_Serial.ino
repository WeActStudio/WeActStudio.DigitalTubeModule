/*
  Basic library example for TM1638. Kept small to show the simplest display functionality.
  Library based on TM1638 library by Ricardo Batista, adapted by Maxint-RD MMOLE 2018.
  
  Tested to work:
      Arduino Nano using Arduino IDE 1.8.2, Nano (Old Bootloader)), 4092 bytes flash, 135 bytes RAM

  For more information see  https://github.com/maxint-rd/TM16xx
*/

#include "hal/uart_types.h"

#define TM1652_SDA_Pin 4
const uint8_t dig_num[] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F};

uint8_t reverseByte(uint8_t b)
{ 
  b = (b & 0b11110000) >> 4 | (b & 0b00001111) << 4;
  b = (b & 0b11001100) >> 2 | (b & 0b00110011) << 2;
  b = (b & 0b10101010) >> 1 | (b & 0b01010101) << 1;
  return b;
}

void tm1652_set_brightness(uint8_t brightness)
{
  uint8_t send[2];
  if (brightness < 0 || brightness > 8)
    return;
  else
  {
    send[0] = 0x18;
    send[1] = 0x10 | (reverseByte(brightness - 1)>>4 & 0x0f);
    Serial1.write(send,2);
  }
}

void tm1652_write_dig(uint8_t dig1,uint8_t dig2,uint8_t dig3,uint8_t dig4)
{
  uint8_t data_write[5];
  data_write[0] = 0x08;
  data_write[1] = dig1;
  data_write[2] = dig2;
  data_write[3] = dig3;
  data_write[4] = dig4;
  Serial1.write(data_write,5);
}

void tm1652_write_num(uint16_t num,uint8_t dot)
{
  uint8_t data_write[5];
  uint8_t temp;
  data_write[0] = 0x08;
  temp = num/1000%10;
  data_write[1] = temp?dig_num[temp]:0x00;
  temp = num/100%10;
  data_write[2] = temp?dig_num[temp]:(data_write[1]?dig_num[temp]:0x00);
  temp = num/10%10;
  data_write[3] = temp?dig_num[temp]:(data_write[2]?dig_num[temp]:0x00);
  temp = num%10;
  data_write[4] = temp?dig_num[temp]:(data_write[3]?dig_num[temp]:0x00);
  if(dot)
    data_write[dot] |= 0x80;
  Serial1.write(data_write,5);
}

void tm1652_clear()
{
  tm1652_write_dig(0,0,0,0);
}

void tm1652_init()
{
  Serial1.begin(19200, SERIAL_8O1, -1, TM1652_SDA_Pin);
  while (!Serial1) {
    delay(10);
  }
  tm1652_clear();
  delay(20);
  tm1652_set_brightness(3);
  delay(20);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(10);
  }

  Serial.print("tm1652 init\r\n");
  tm1652_init();

  Serial.print("tm1652 write num\r\n");
  tm1652_write_num(2024,0);
  
  delay(1000);
}

void loop() {
  tm1652_write_num(2024,2);
  delay(1000);
  tm1652_write_num(2024,0);
  delay(1000);
}

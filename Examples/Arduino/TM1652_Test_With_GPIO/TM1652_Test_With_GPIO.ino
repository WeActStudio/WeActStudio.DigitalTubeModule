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

void tm1652_send(uint8_t data)
{	// Send a byte to the chip the way the TM1652 likes it (LSB-first, UART serial 8E1 - 8 bits, parity bit set to 0 when odd, one stop bit)
	// Note: while segment data is LSB-first, address bits and SEG/GRID intensity bit are reversed
	// The address command can be followed by multiple databytes, requiring specific timing to distinguish multiple commands
  //  - start bit, 8x data bits, parity bit, stop bit; 52 us = 19200bps
  #define TM1652_BITDELAY 49     // NOTE: core 1.0.6 of LGT8F328@32MHz miscalculates delayMicroseconds() (should be 52us delay). For fix see https://github.com/dbuezas/lgt8fx/issues/18
  bool fParity=true;

  noInterrupts();

  // start - low
  digitalWrite(TM1652_SDA_Pin, LOW);
  delayMicroseconds(TM1652_BITDELAY);

  // data low-0; high=1
  for(int nBit=0; nBit<8; nBit++)
  {
    if(data&1) fParity=!fParity;
    digitalWrite(TM1652_SDA_Pin, (data&1) ? HIGH : LOW);
    data>>=1;
    delayMicroseconds(TM1652_BITDELAY);
  }

  // parity - low when odd
  digitalWrite(TM1652_SDA_Pin, fParity);
  delayMicroseconds(TM1652_BITDELAY);

  // stop - high
  digitalWrite(TM1652_SDA_Pin, HIGH);
  interrupts();

  delayMicroseconds(TM1652_BITDELAY);
  // idle - remain high
  delayMicroseconds(TM1652_BITDELAY);
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
    tm1652_send(send[0]);
    tm1652_send(send[1]);
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
  tm1652_send(data_write[0]);
  tm1652_send(data_write[1]);
  tm1652_send(data_write[2]);
  tm1652_send(data_write[3]);
  tm1652_send(data_write[4]);
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
  tm1652_send(data_write[0]);
  tm1652_send(data_write[1]);
  tm1652_send(data_write[2]);
  tm1652_send(data_write[3]);
  tm1652_send(data_write[4]);
}

void tm1652_clear()
{
  tm1652_write_dig(0,0,0,0);
}

void tm1652_init()
{
  pinMode(TM1652_SDA_Pin, OUTPUT);
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

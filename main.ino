#include <Arduino.h>

/**************************************************************************
 * MAX31865 Basic Example
 *
 * Copyright (C) 2015 Ole Wolf <wolf@blazingangles.com>
 *
 *
 * Example code that reads the temperature from an MAX31865 and outputs
 * it on the serial line.
 * 
 * Wire the circuit as follows, assuming that level converters have been
 * added for the 3.3V signals:
 *
 *    Arduino Uno   -->  MAX31865
 *    ---------------------------
 *    CS: pin 10    -->  CS
 *    MOSI: pin 11  -->  SDI (must not be changed for hardware SPI)
 *    MISO: pin 12  -->  SDO (must not be changed for hardware SPI)
 *    SCK: pin 13   -->  SCLK (must not be changed for hardware SPI)
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************************/

#include <SPI.h>
#include <MAX31865.h>

#include <EEPROM.h>

#include <My.h>

#define RTD_CS_PIN 5

#define EEPROM_ADDRESS_START 0;

MAX31865_RTD rtd(MAX31865_RTD::RTD_PT100, RTD_CS_PIN);
uint16_t raw_0;
uint16_t raw_100;

My my;

////按钮响应，切换模式/////////
enum
{
  MODE_RUN,
  MODE_SET_RAW_0,
  MODE_SET_RAW_100,
  MODE_END
};
static uint8_t Mode = MODE_RUN;
void onButton()
{
  Mode++;

  if (Mode > MODE_END)
  {
    Mode = MODE_RUN;
  }

  switch (Mode)
  {
  case MODE_RUN:
    Serial.println("mode run");
    break;

  case MODE_SET_RAW_0:
    Serial.println("setting 0C");
    break;

  case MODE_SET_RAW_100:
    Serial.println("save 0C raw, setting 100C");
    break;

  case MODE_END:
    Serial.println("save raw 100C, end.");
    break;

  default:
    break;
  }
}

//////////////双击按钮///////////////
struct Button
{
  const uint8_t PIN;
  unsigned long preMillis;
};

Button button = {0, 100000};

void IRAM_ATTR isr()
{

  unsigned long currentMillis = millis();
  unsigned long diff = abs(currentMillis - button.preMillis);

  if (Mode != MODE_RUN)
  {                 //非运行模式
    if (diff < 200) //防抖
    {
      return;
    }
    onButton(); //切换到下一个模式
    return;
  }

  if (diff > 1500) //超期,更新时间
  {
    button.preMillis = currentMillis;
    Serial.println("first press");
    return;
  }
  else if (diff < 200) //防抖
  {
    return;
  }
  else
  {
    //双击
    Serial.println("double press");
    button.preMillis = millis();

    onButton(); //切换模式
  }
}
/////////////////////////////////////////

void setup()
{
  Serial.begin(9600);

  pinMode(button.PIN, INPUT_PULLUP);
  attachInterrupt(button.PIN, isr, FALLING);

  /* Initialize SPI communication. */
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE3);

  if (!EEPROM.begin(1000))
  { //初始化大小
    Serial.println("Failed to initialise EEPROM");
    Serial.println("Restarting...");
    delay(1000);
    ESP.restart();
  }

  int address = 0;

  EEPROM.writeUInt(address, 7672);
  address += sizeof(uint16_t);

  EEPROM.writeUInt(address, 10573);
  address += sizeof(uint16_t);

  EEPROM.commit(); //等待写入完成

  address = 0;
  raw_0 = EEPROM.readUInt(address);
  address += sizeof(uint16_t);

  raw_100 = EEPROM.readUInt(address);
  address += sizeof(uint16_t);

  Serial.print("raw 0=");
  Serial.print(raw_0);
  Serial.print("  raw 100=");
  Serial.print(raw_100);
  Serial.println();

  /* Allow the MAX31865 to warm up. */
  delay(100);

  /* Configure:

       V_BIAS enabled
       Auto-conversion
       1-shot disabled
       3-wire enabled
       Fault detection:  automatic delay
       Fault status:  auto-clear
       50 Hz filter
       Low threshold:  0x0000
       High threshold:  0x7fff
  */
  rtd.configure(true, true, false, true, MAX31865_FAULT_DETECTION_NONE,
                true, true, 0x0000, 0x7fff);
}

void loop()
{

  //结束后0.5秒切换RUN模式，避免按键按太快，逻辑出现混乱
  if (Mode == MODE_END)
  {
    ///TODO：save raw 100


    ///
    delay(500);
    onButton();
  }

  rtd.read_all();

  if (rtd.status() == 0)
  {

    uint16_t raw = rtd.raw_resistance();
    Serial.print("raw=");
    Serial.println(raw);

    Serial.print("T=");
    Serial.print(my.temperature(raw_0, raw_100, raw), 1);
    Serial.println(" C");
    Serial.println();
  }
  else
  {
    Serial.print("RTD fault register: ");
    Serial.print(rtd.status());
    Serial.print(": ");
    if (rtd.status() & MAX31865_FAULT_HIGH_THRESHOLD)
    {
      Serial.println("RTD high threshold exceeded");
    }
    else if (rtd.status() & MAX31865_FAULT_LOW_THRESHOLD)
    {
      Serial.println("RTD low threshold exceeded");
    }
    else if (rtd.status() & MAX31865_FAULT_REFIN)
    {
      Serial.println("REFIN- > 0.85 x V_BIAS");
    }
    else if (rtd.status() & MAX31865_FAULT_REFIN_FORCE)
    {
      Serial.println("REFIN- < 0.85 x V_BIAS, FORCE- open");
    }
    else if (rtd.status() & MAX31865_FAULT_RTDIN_FORCE)
    {
      Serial.println("RTDIN- < 0.85 x V_BIAS, FORCE- open");
    }
    else if (rtd.status() & MAX31865_FAULT_VOLTAGE)
    {
      Serial.println("Overvoltage/undervoltage fault");
    }
    else
    {
      Serial.println("Unknown fault; check connection");
    }
  }

  delay(3000);
}

#include <Arduino.h>
#include <SPI.h>
#include <EEPROM.h>
#include <./MAX31865/MyTemperature.h>

#define CS_PIN 5

#define EEPROM_ADDRESS_START 0;

uint16_t raw_0;
uint16_t raw_100;

MyTemperature *my;


void setup()
{
  Serial.begin(9600);
  raw_0 = 7672;
  raw_100 = 10573L;

  //////////////////////////////////////////////////////////////
  /* Initialize SPI communication. */
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE3);
  /* Allow the MAX31865 to warm up. */
  delay(100);
  
  my = new MyTemperature(CS_PIN, raw_0, raw_100);
  /////////////////////////////////////////////////////////////
}

void loop()
{
  uint16_t raw = my->getRawResistance();
  Serial.print("raw=");
  Serial.println(raw);

  double t = my->getTemperature();
  Serial.print("T=");
  Serial.print(t, 1);
  Serial.println(" C");
  Serial.println(); 

  delay(3000);
}

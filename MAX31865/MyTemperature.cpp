#include <Arduino.h>
#include <./MAX31865/MyTemperature.h>

MyTemperature::MyTemperature(uint8_t CS_PIN, uint16_t raw_0, uint16_t raw_100) : raw_0(raw_0), raw_100(raw_100)
{
    rtd = new MAX31865_RTD(MAX31865_RTD::RTD_PT100, CS_PIN);

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
    // rtd.configure(true, true, false, true, MAX31865_FAULT_DETECTION_NONE,
    //               true, true, 0x0000, 0x7fff);
    rtd->configure(true, true, false, true, MAX31865_FAULT_DETECTION_NONE,
                   true, true, 0x0000, 0x7fff);
}

uint16_t MyTemperature::getRawResistance()
{
    rtd->read_all();//read all
    return rtd->raw_resistance();
}

void MyTemperature::setRaw_0_100(uint16_t raw_0, uint16_t raw_100)
{
    this->raw_0 = raw_0;
    this->raw_100 = raw_100;
}


double MyTemperature::getTemperature()
{

    if ((raw_100 - raw_0) < 100) //初始参数错误,或者未曾初始化
    {
        return -300;
    }

    uint16_t raw_resistance = getRawResistance();

    double rate100 = raw_100 - raw_0;    //注意int转double，不要简写
    rate100 = rate100 / 100;             //0-100斜率
    double rate50 = rate100 * RATE_0_50; //0-50斜率
    double temperature2 = raw_resistance - raw_0;
    temperature2 = temperature2 / rate50;

    if (temperature2 >= 50) //超过50度，重新计算
    {
        uint16_t rt50 = raw_0 + rate50 * 50;       //计算50度时对应的raw值
        double rate50_100 = rate100 * RATE_50_100; //50到100的斜率
        temperature2 = 50 + (raw_resistance - rt50) / rate50_100;
    }

    if (temperature2 >= 100) //超过100度，重新计算
    {
        double rate100_150 = rate100 * RATE_100_150; //100到150的斜率
        temperature2 = 100 + (raw_resistance - raw_100) / rate100_150;
    }

    if (temperature2 >= 150)
    {
        double rate100_150 = rate100 * RATE_100_150; //100到150的斜率
        uint16_t rt150 = raw_100 + rate100_150 * 50; //计算150度时对应的raw值
        double rate150_200 = rate100 * RATE_150_200; //150到200的斜率
        temperature2 = 150 + (raw_resistance - rt150) / rate150_200;

        if (temperature2 >= 200)
        {
            uint16_t rt200 = rt150 + rate150_200 * 50;   //计算200度时对应的raw值
            double rate200_250 = rate100 * RATE_200_250; //200-250的斜率
            temperature2 = 200 + (raw_resistance - rt200) / rate200_250;
        }
    }

    return temperature2;
}

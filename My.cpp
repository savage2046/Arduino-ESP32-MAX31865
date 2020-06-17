#include <Arduino.h>
#include <My.h>

My::My()
{
}

double My::temperature(uint16_t raw_0, uint16_t raw_100, uint16_t raw_resistance)
{

    if ((raw_100 - raw_0) < 100) //初始参数错误,或者未曾初始化
    {
        return -300;
    }

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

    if (temperature2 >= 100)//超过100度，重新计算
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

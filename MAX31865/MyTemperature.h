#ifndef _MyTemperature_H
#define _MyTemperature_H

#include <stdint.h>
#include <./MAX31865/MAX31865.h>
/**
 * MAX31865参考电阻应该用400欧不知道为什么买来的模块都是用430欧
 * 而且把设置改成430后还是不准
 * 
 * 于是自行标定0度和100度对应的原始数据 * 
 * 根据PT100给出的温度-电阻对应表，以50度为一个阶梯，直接用原始数据计算温度 
 * 
 * 以下是各温度阶段斜率和0-100度斜率的比值（0-250度）
 * 
**/
#define RATE_0_50 1.007531;
#define RATE_50_100 0.992469;
#define RATE_100_150 0.977408;
#define RATE_150_200 0.962347;
#define RATE_200_250 0.947286;

class MyTemperature
{

private:
    MAX31865_RTD *rtd;
    uint16_t raw_0;
    uint16_t raw_100;

public:
    MyTemperature(uint8_t CS_PIN, uint16_t raw_0, uint16_t raw_100);
    uint16_t getRawResistance();
    double getTemperature();
    void setRaw_0_100(uint16_t raw_0, uint16_t raw_100);
};

#endif
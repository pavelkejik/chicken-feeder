
/***********************************************************************
 * Filename: motor.cpp
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Implements the Motor class.
 * 
 ***********************************************************************/


#include "Arduino.h"
#include "common.h"
#include "motor.h"
#include "pin_map.h"
#include "parameters.h"
#include "error.h"

void Motor::ReadCurrent(void)
{
    int32_t curr = analogReadMilliVolts(pinISense);
    AktualniProud_mA.Set(current.Input(curr));
    // Serial.println(curr);
}

void ReadBatteryVoltage(void)
{
    digitalWrite(BAT_NSENSE, LOW);
    delay(5);
    uint32_t bat_v = analogReadMilliVolts(BAT_AINP);
    delay(1);
    bat_v += analogReadMilliVolts(BAT_AINP);
    bat_v *= (BAT_RDIVIDER_R_UP + BAT_RDIVIDER_R_DOWN);
    bat_v /= (BAT_RDIVIDER_R_DOWN * 2);

    int32_t last_bat_v = (uint16_t)NapetiBaterie_mV.Get() & 0x7FFF;
    if (last_bat_v != 0)
    {
                bat_v = last_bat_v + ((int32_t)bat_v - last_bat_v) / 8;
    }
    if (!digitalRead(LBO))
    {
        bat_v = bat_v | (1 << 15);
    }
    NapetiBaterie_mV.Set((int32_t)bat_v);
    // Serial.println(NapetiBaterie_mV.Get());
    digitalWrite(BAT_NSENSE, HIGH);
}

void Motor::Init(void)
{
    pinMode(pinOpen, OUTPUT);
    pinMode(pinClose, OUTPUT);
    pinMode(pinSleep, OUTPUT);
    pinMode(BAT_NSENSE, OUTPUT);
    pinMode(LBO, INPUT_PULLUP);

    digitalWrite(BAT_NSENSE, HIGH);

    digitalWrite(pinSleep, HIGH);
    analogSetPinAttenuation(pinISense, ADC_0db);
    adcAttachPin(pinISense);
    adcAttachPin(BAT_AINP);
    ReadBatteryVoltage();
    Stop();
}

void Motor::Open(void)
{
    state = motor_to_open;
    digitalWrite(pinSleep, HIGH);
    delay(10);

    Error::ClearActionAll(MOVE_UP | MOVE_DOWN);
    digitalWrite(pinOpen, HIGH);
    digitalWrite(pinClose, LOW);
}

void Motor::Close(void)
{
    state = motor_to_close;
    digitalWrite(pinSleep, HIGH);
    delay(10);
    Error::ClearActionAll(MOVE_UP | MOVE_DOWN);
    digitalWrite(pinOpen, LOW);
    digitalWrite(pinClose, HIGH);
}

void Motor::StopFast(void)
{
    digitalWrite(pinOpen, HIGH);
    digitalWrite(pinClose, HIGH);
    delay(10);
    digitalWrite(pinSleep, LOW);
    state = motor_faststop;
}

void Motor::Stop(void)
{
    digitalWrite(pinOpen, LOW);
    digitalWrite(pinClose, LOW);
    delay(10);
    digitalWrite(pinSleep, LOW);
    state = motor_stop;
}

void Motor::Sleep(void)
{
    digitalWrite(pinSleep, LOW);
}

void Motor::Run(void)
{
    ReadCurrent();

    switch (state)
    {
    case motor_to_open:
        ovrl_open.Check(AktualniProud_mA.Get() > MotorMaxProud_mA.Get());
        ovrl_close.Check(false);
        no_current.Check(AktualniProud_mA.Get() < ZERO_CURRENT_THRESHOLD_mV);
        break;

    case motor_to_close:
        ovrl_close.Check(AktualniProud_mA.Get() > MotorMaxProud_mA.Get());
        ovrl_open.Check(false);
        no_current.Check(AktualniProud_mA.Get() < ZERO_CURRENT_THRESHOLD_mV);
        break;

    default:
        ovrl_open.Check(false);
        ovrl_close.Check(false);
        no_current.Check(false);
        break;
    }
}

bool Motor::IsLowBattery(void)
{
    return NapetiBaterie_mV.Get() < LOW_BATTERY_THRESHOLD;
}

bool Motor::IsStopped(void)
{
    return (state == motor_stop) || (state == motor_faststop);
}
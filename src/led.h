/***********************************************************************
 * Filename: led.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Declares the Led class to manage LED control patterns and 
 *     operations. It includes methods for initialization, setting 
 *     control patterns, and executing control tasks.
 *
 ***********************************************************************/


#pragma once

#include "Arduino.h"

#define LED_CTRL_TIME_MS 30
#define LED_DIMMING_STEP 2

class Led
{
private:
	uint16_t CtrlPattern;
	uint8_t CtrlStep;
	uint8_t pinNmr;
	bool dim_up;
	uint8_t act_duty;
	uint8_t dem_duty;
	const int freq = 5000;
	const int ledChannel = 0;
	const int resolution = 8;
	bool dim = false;

	static const uint8_t dim_table[];

public:
	Led(uint8_t pin) : CtrlPattern(0), pinNmr(pin)
	{
	}
	void Init(void)
	{
		// pinMode(pinNmr, OUTPUT);
		ledcSetup(ledChannel, freq, resolution);
		ledcAttachPin(pinNmr, ledChannel);
		act_duty = 0;
		SetDimming(255);
	}
	void SetCtrl(uint16_t Pattern)
	{
		if (Pattern != CtrlPattern)
		{
			CtrlPattern = Pattern;
			CtrlStep = 0;
		}
	}
	void CtrlTask(void)
	{
		if (dim)
		{
			Dimming();
		}
		else
		{
			if (++CtrlStep >= 32)
			{
				CtrlStep = 0;
			}
			if (CtrlStep < 32)
			{
				ledcWrite(ledChannel, ((CtrlPattern & (1 << (CtrlStep/2))) != 0) ? 255 : 0);
			}
		}
	}
	Led &operator=(uint16_t in)
	{
		SetCtrl(in);
		SetDimming(false);
		return *this;
	}
	bool Dimming(void)
	{
		if (act_duty >= 16)
		{
			act_duty = 0;
			if (++CtrlStep >= 8)
			{
				CtrlStep = 0;
			}
		}
		else
		{
			act_duty++;
		}

		if (((CtrlPattern & (1 << CtrlStep)) != 0))
		{
			ledcWrite(ledChannel, dim_table[act_duty & 0x0F]);
		}
		else
		{
			ledcWrite(ledChannel, 0);
		}
		return act_duty == 0x0F;
	}

	void SetDimming(bool on = true)
	{
		dim = on;
	}

	void Sleep(void)
	{
	}
};
extern Led LedR;

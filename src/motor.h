
/***********************************************************************
 * Filename: motor.h
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     Declares the Motor class which handles the control and monitoring
 *     of a motor, including functionalities for opening, closing, stopping,
 *     and monitoring for overcurrent. 
 * 
 ***********************************************************************/


#pragma once
#include "Arduino.h"
#include "exp_filter.h"

// #define DEBUG_SERVO_INFO

#define ZERO_CURRENT_THRESHOLD_mV 25
#define MOTOR_CONTROL_TASK_PERIOD_MS 50

#define BAT_RDIVIDER_R_UP 100
#define BAT_RDIVIDER_R_DOWN 100

#define LOW_BATTERY_THRESHOLD 3200


typedef enum
{
	motor_stop = 0,
	motor_faststop = 1,
	motor_to_open = 2,
	motor_to_close = 3,
} MotorState_t;

class Motor
{
private:
	const int pinOpen;
	const int pinClose;
	const int pinSleep;
	const int pinISense;
	MotorState_t state;
	ExpFilter current;

public:
	Motor(int pOpen, int pClose, int pSleep, int pISense) : pinOpen(pOpen), pinClose(pClose), pinSleep(pSleep), pinISense(pISense){};

	void Init(void);
	void Open(void);
	void Close(void);
	void StopFast(void);
	void Stop(void);

	bool IsStopped(void);

	void IsOverCurrentOpen(int16_t current_ma);
	void IsOverCurrentClose(int16_t current_ma);
	void ReadCurrent(void);

	bool IsLowBattery(void);

	void Sleep(void);

	void Run(void);
};
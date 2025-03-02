/***********************************************************************
 * Filename: main.cpp
 * Author: Pavel Kejik
 * Date: 2024-04-12
 * Description:
 *     This file serves as the entry point for the application, 
 *     setting up and managing all hardware and software components. 
 *     It initializes modules such as ESP-NOW as well as handling device management and logging 
 *     The program controls various tasks for feeder management, system logging and 
 *     deep sleep control, incorporating extensive use of FreeRTOS for task management.
 *
 ***********************************************************************/

#include <Arduino.h>
#include "button.h"
#include "led.h"
#include "pin_map.h"
#include "motor.h"
#include "parameters.h"
#include "error.h"
#include "log.h"
#include "feeder_ctrl.h"
#include "common.h"
#include <esp_now.h>
#include "esp32c3/rom/rtc.h"
#include <LittleFS.h>
#include "esp_now_client.h"
#include "time_ctrl.h"
#include "FreeRTOSConfig.h"
#include "esp_freertos_hooks.h"
#include "deep_sleep_ctrl.h"
#include "weight.h"

#define TIME_SCHEDULE(_t_secs) ((uint32_t)((_t_secs) * 1000 / COMMON_LOOP_TASK_PERIOD_MS))

Button btn1(BTN_1, INPUT);
Button btn2(BTN_2, INPUT);
Led LedR(LED_R);
Motor motor(MOT_A1, MOT_A2, MOT_SLEEP, MOT_ISENSE);
Weight weight;

static uint32_t btn_timer;
void MotorTask(void *pvParameters)
{
  while (true)
  {
    motor.Run();
    delay(MOTOR_CONTROL_TASK_PERIOD_MS);
  }
}

void FeederControlTask(void *pvParameters)
{
  while (true)
  {
    FeederCtrl::Task();
    delay(FEEDER_CONTROL_TASK_PERIOD_MS);
  }
}

void LedControlTask(void *pvParameters)
{
  delay(100);
  LedR = 0;
  delay(500);
  while (true)
  {
    if (motor.IsLowBattery())
    {
      LedR = 0x11;
    }
    else if (StavZarizeni.Get() == Vybrano)
    {
      LedR = 0xff;
      LedR.SetDimming();
    }
    else if (StavZarizeni.Get() == Parovani)
    {
      LedR = 0xff;
      LedR.SetDimming();
    }
    else if (StavZarizeni.Get() == Sparovano)
    {
      LedR = 0;
      delay(500);
      LedR = 0x0CCC;
      StavZarizeni.Set(NormalniMod);
      delay(800);
      LedR = 0;
      delay(2000);
    }
    else
    {
      switch (StavKrmitka.Get())
      {
      case NeznaznamaPoloha:
        LedR = 0;
        break;
      case Otevirani:
      case Zavirani:
      case UvolneniOtevirani:
      case UvolneniZavirani:
        LedR = 0x1111;
        break;
      case Otevreno:
      case Zavreno:
        LedR = 0;
        break;
      case StopZavirani:
      case StopOtevirani:
        LedR = 0;
        break;
      case ChybaKrmitka:
        LedR = 0x01;
        break;
      default:
        break;
      }
    }
    delay(100);
  }
}

void ledUpdate(void)
{
  LedR.CtrlTask();
}

void SystemLogTask(void *pvParameters)
{
  while (true)
  {
    SystemLog::Task();

    delay(200);
  }
}

void ESPNowTask(void *pvParameters)
{
  while (true)
  {
    ESPNowCtrl::Task();
    // delayMicroseconds(100);
  }
}

void ESPNowSlaveTask(void *pvParameters)
{
  while (true)
  {
    ESPNowClient::Task();
  }
}

void TimeControlTask(void *pvParameters)
{
  while (true)
  {
    TimeCtrl::Task();
    delay(TIME_CONTROL_TASK_PERIOD_MS);
  }
}

void CommandProcessingTask(void *pvParameters)
{
  while (true)
  {
    active_tasks[Write_Command_Task] = true;
    weight.Task();
    active_tasks[Write_Command_Task] = false;
    xSemaphoreTake(write_cmd_sem, pdMS_TO_TICKS(1000));
  }
}


void SleepTask(void *pvParameters)
{
  while (true)
  {
    if (IsSystemIdle())
    {
      vTaskSuspendAll();
      for (int i = 0; i < NUMBER_TASK_HANDLES; i++)
      {
        if (active_task_handle[i] != 0)
        {
          vTaskDelete(active_task_handle[i]);
        }
      }
      xTaskResumeAll();
      Register::Sleep();
      motor.Sleep();
      SystemLog::Sleep();
      if (RestartCmd.Get() == povoleno)
      {
        ESP.restart();
      }
      else
      {
        ESPNowClient::Sleep();
        esp_sleep_enable_timer_wakeup(PeriodaKomunikace_S.Get() * 1000000ULL);
        pinMode(BTN_WAKE, INPUT);
        esp_deep_sleep_enable_gpio_wakeup(BIT(BTN_WAKE), ESP_GPIO_WAKEUP_GPIO_LOW);
        gpio_hold_en((gpio_num_t)PDCLK);
        gpio_deep_sleep_hold_en();
        esp_deep_sleep_start();
      }
    }
    delay(10);
  }
}

void setup()
{
  Serial.begin(115200);
  Register::InitAll();
  storageFS.begin(true, "/storage", 5);

  SystemLog::Init();
  Error::ClearAll();
  btn1.Init();
  btn2.Init();
  LedR.Init();
  motor.Init();
  weight.Init();
  FeederCtrl::Init();
  TimeCtrl::Init();
  ESPNowClient::Init();
  active_tasks[Button_Task] = false;

  switch (rtc_get_reset_reason(0))
  {
  case 1:
    ResetReason.Set(rst_Poweron);
    break;
  case 3:
  case 12:
    ResetReason.Set(rst_Software);
    break;
  case 5:
    ResetReason.Set(rst_Deepsleep);
    break;
  case 4:
  case 7:
  case 8:
  case 9:
  case 11:
  case 13:
  case 16:
    ResetReason.Set(rst_Watchdog);
    break;
  case 15:
    ResetReason.Set(rst_Brownout);
    break;

  case 14:
    ResetReason.Set(rst_External);

  default:
    ResetReason.Set(rst_Unknown);
  }

  if (esp_sleep_get_gpio_wakeup_status())
  {
    active_tasks[Button_Task] = true;
    StartTimer(btn_timer, TIME_SCHEDULE(10));
    LedR = 0xffff;
  }

  memset(active_task_handle, 0, sizeof(active_task_handle));

  xTaskCreateUniversal(MotorTask, "motorTask", getArduinoLoopTaskStackSize(), NULL, 4, &active_task_handle[0], ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(FeederControlTask, "feederCtrlTask", getArduinoLoopTaskStackSize(), NULL, 2, &active_task_handle[1], ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(LedControlTask, "ledCtrlTask", getArduinoLoopTaskStackSize(), NULL, 1, &active_task_handle[2], ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(SystemLogTask, "logTask", getArduinoLoopTaskStackSize(), NULL, 1, &active_task_handle[3], ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(CommandProcessingTask, "cmdTask", getArduinoLoopTaskStackSize(), NULL, 1, &active_task_handle[4], ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(TimeControlTask, "timeCtrlTask", getArduinoLoopTaskStackSize(), NULL, 2, &active_task_handle[5], ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(ESPNowTask, "espNowTask", getArduinoLoopTaskStackSize(), NULL, 5, NULL, ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(ESPNowSlaveTask, "espNowSlaveTask", getArduinoLoopTaskStackSize(), NULL, 1, &active_task_handle[6], ARDUINO_RUNNING_CORE);
  xTaskCreateUniversal(SleepTask, "sleepTask", getArduinoLoopTaskStackSize(), NULL, 1, NULL, ARDUINO_RUNNING_CORE);

  // esp_register_freertos_idle_hook(IdleTask);

  hw_timer_t *timer = NULL;
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &ledUpdate, true);
  timerAlarmWrite(timer, LED_CTRL_TIME_MS * 1000, true);
  timerAlarmEnable(timer);
}

void loop()
{
  btn1.Read();
  btn2.Read();

  if (btn1.IsOnEvent())
  {
    StartTimer(btn_timer, TIME_SCHEDULE(10));
    // SystemLog::Print();
    switch (StavKrmitka.Get())
    {
    case Otevreno:
    case NeznaznamaPoloha:
      FeederCtrl::EventSourced(ev_close, povel_tlacitkem);
      break;

    case Zavreno:
      break;

    default:
      FeederCtrl::Event(ev_stop);
      break;
    }
  }

  if (btn2.IsOnEvent())
  {
    active_tasks[Button_Task] = true;
    StartTimer(btn_timer, TIME_SCHEDULE(10));
    // SystemLog::Print();
    switch (StavKrmitka.Get())
    {
    case Zavreno:
    case NeznaznamaPoloha:
      FeederCtrl::EventSourced(ev_open, povel_tlacitkem);
      break;

    case Otevreno:
      break;

    default:
      FeederCtrl::Event(ev_stop);
      break;
    }
  }

  if (btn2.IsLongOnEvent())
  {
    active_tasks[Button_Task] = true;
    StartTimer(btn_timer, TIME_SCHEDULE(60));
    ESPNowClient::StartPairing();
  }

  if (EndTimer(btn_timer))
  {
    active_tasks[Button_Task] = false;
  }
    delay(COMMON_LOOP_TASK_PERIOD_MS);
}
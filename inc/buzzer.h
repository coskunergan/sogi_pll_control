/*
    Buzzer Lib

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/

#if !defined(__INCLUDE_BUZZER_H__)
#define __INCLUDE_BUZZER_H__

#include "RTE_Components.h"
#include  CMSIS_device_header
#include <chrono>
#include "mutex.h"
#include "timer.h"
#include "gpio_hal.h"

using namespace gpio_hal;

GpioOutput buzz_pin;

class buzzer
{
public:
    void init()
    {
        buzz_pin.init(buzz_pin.make_pin(GPIOA, GPIO_Pin_11), true);
    }

    void beep(std::chrono::milliseconds(msec))
    {
        if(!m_tim_ptr || (!m_tim_ptr->running()))
        {
            sys::timer buzz_timer(std::chrono::milliseconds(msec), [&]
            {
                buzz_pin.off();
                return false;
            });
            m_tim_ptr = std::make_unique<sys::timer>(std::move(buzz_timer));
            buzz_pin.on();
            m_tim_ptr->start();
        }
    }

private:
    std::unique_ptr<sys::timer> m_tim_ptr;
};

#endif

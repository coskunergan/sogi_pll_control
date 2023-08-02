/*
    Buzzer Lib

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/

#pragma once
#include "RTE_Components.h"
#include  CMSIS_device_header
#include <chrono>
#include "timer.h"
#include "gpio_hal.h"

namespace device_buzzer
{
    using namespace gpio_hal;
    class buzzer
    {
    public:
        buzzer() = default;
        ~buzzer() = default;
        buzzer(buzzer &&) = delete;
        buzzer &operator=(buzzer &&) = delete;
        buzzer(const buzzer &) = delete;
        buzzer &operator=(const buzzer &) = delete;

        void init()
        {
            buzz_pin.init(GPIOA, GPIO_Pin_11, true);
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
        GpioOutput buzz_pin;
        std::unique_ptr<sys::timer> m_tim_ptr;
    };
}

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
        buzzer(GPIO_TypeDef *const port, const uint16_t gpio, const bool activeLow = false) : m_port(port), m_gpio(gpio), m_activeLow(activeLow)
        {
            buzzer::buzz_pin.init(m_port, m_gpio, m_activeLow);
        }
        ~buzzer() = default;
        buzzer(buzzer &&) = delete;
        buzzer &operator=(buzzer &&) = delete;
        buzzer(const buzzer &) = delete;
        buzzer &operator=(const buzzer &) = delete;

        void beep(std::chrono::milliseconds(msec))
        {
            if(!m_tim_ptr || (!m_tim_ptr->running()))
            {
                sys::timer buzz_timer(std::chrono::milliseconds(msec), [&]
                {
                    buzzer::buzz_pin.off();
                    return false;
                });
                m_tim_ptr = std::make_unique<sys::timer>(std::move(buzz_timer));
                buzzer::buzz_pin.on();
                m_tim_ptr->start();
            }
            else
            {
                m_tim_ptr->stop();
                sys::timer buzz_timer(std::chrono::milliseconds(msec), [&]
                {
                    buzzer::buzz_pin.off();
                    return false;
                });
                m_tim_ptr = std::make_unique<sys::timer>(std::move(buzz_timer));
                buzzer::buzz_pin.on();
                m_tim_ptr->start();
            }
        }
    private:
        GPIO_TypeDef *const m_port;
        const uint16_t m_gpio;
        const bool m_activeLow;
        static GpioOutput buzz_pin;
        std::unique_ptr<sys::timer> m_tim_ptr;
    };
    GpioOutput buzzer::buzz_pin;
}

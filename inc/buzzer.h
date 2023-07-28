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

GPIO_TypeDef *const Buzzer_Port = GPIOA;
const uint16_t Buzzer_Pin       = GPIO_Pin_11;
const uint32_t Buzzer_Clk       = RCC_AHBPeriph_GPIOA;

class buzzer
{
public:
    void init()
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
        GPIO_InitStructure.GPIO_Pin = Buzzer_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
        GPIO_Init(Buzzer_Port, &GPIO_InitStructure);
        GPIO_SetBits(Buzzer_Port, Buzzer_Pin);// off
    }

    void deinit()
    {
        GPIO_InitTypeDef GPIO_InitStructure;
        GPIO_InitStructure.GPIO_Pin = Buzzer_Pin;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
        GPIO_Init(Buzzer_Port, &GPIO_InitStructure);
    }

    void beep(std::chrono::milliseconds(msec))
    {
        if(!m_tim_ptr || (!m_tim_ptr->running()))
        {
            sys::timer buzz_timer(std::chrono::milliseconds(msec), [&]
            {
                return beep_off();
            });
            m_tim_ptr = std::make_unique<sys::timer>(std::move(buzz_timer));
            beep_on();
            m_tim_ptr->start();
        }
    }

private:
    std::unique_ptr<sys::timer> m_tim_ptr;

    void beep_on()
    {
        GPIO_ResetBits(Buzzer_Port, Buzzer_Pin);
    }

    bool beep_off()
    {
        GPIO_SetBits(Buzzer_Port, Buzzer_Pin);
        return false;
    }
};

#endif

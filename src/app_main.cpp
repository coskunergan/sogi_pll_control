/*
    Main Program

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/

#include "RTE_Components.h"
#include  CMSIS_device_header
#include "printf_io.h"
#include "os.h"
#include "thread.h"
#include "timer.h"
#include "mutex.h"
#include "chrono.h"
#include "osexception.h"
#include "button.h"
#include "buzzer.h"
#include <atomic>

using namespace cmsis;

button butt;
buzzer buzz;
std::atomic_int enc_count;

enum : size_t
{
    button_id = 0,
    encoder_button_id = 1
};

const uint16_t Button_Pin         = GPIO_Pin_0;
GPIO_TypeDef *const Button_Port   = GPIOA;
const uint32_t Button_Clk         = RCC_AHBPeriph_GPIOA;
const uint8_t Button_Source       = GPIO_PinSource0;
const uint8_t Button_ExtiPin      = EXTI_PinSource0;
const uint8_t Button_ExtiPort     = EXTI_PortSourceGPIOA;
const uint32_t Button_ExtiLine    = EXTI_Line0;
const IRQn_Type Button_IRQn       = EXTI0_IRQn;

const uint16_t EncoderA_Pin       = GPIO_Pin_2;
GPIO_TypeDef *const EncoderA_Port = GPIOA;
const uint32_t EncoderA_Clk       = RCC_AHBPeriph_GPIOA;
const uint8_t EncoderA_Source     = GPIO_PinSource2;
const uint8_t EncoderA_ExtiPin    = EXTI_PinSource2;
const uint8_t EncoderA_ExtiPort   = EXTI_PortSourceGPIOA;
const uint32_t EncoderA_ExtiLine  = EXTI_Line2;
const IRQn_Type EncoderA_IRQn     = EXTI2_IRQn;

const uint16_t EncoderB_Pin       = GPIO_Pin_4;
GPIO_TypeDef *const EncoderB_Port = GPIOB;
const uint32_t EncoderB_Clk       = RCC_AHBPeriph_GPIOB;

extern "C" void EXTI0_IRQHandler(void)
{
    butt.isr_handler(0);
    EXTI_ClearITPendingBit(EXTI_Line0);
}

extern "C" void EXTI2_IRQHandler(void)
{
    butt.isr_handler(1);
    EXTI_ClearITPendingBit(EXTI_Line2);
}

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void pre_init()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    printf_init();
    //------------------------------
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    SYSCFG_EXTILineConfig(EncoderA_ExtiPort, EncoderA_ExtiPin);
    SYSCFG_EXTILineConfig(Button_ExtiPort, Button_ExtiPin);

    RCC_AHBPeriphClockCmd(EncoderA_Clk, ENABLE);
    RCC_AHBPeriphClockCmd(EncoderB_Clk, ENABLE);
    RCC_AHBPeriphClockCmd(Button_Clk, ENABLE);

    GPIO_InitStructure.GPIO_Pin = Button_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(Button_Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = EncoderA_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(EncoderA_Port, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = EncoderB_Pin;
    GPIO_Init(EncoderB_Port, &GPIO_InitStructure);

    EXTI_InitStructure.EXTI_Line = Button_ExtiLine;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    EXTI_InitStructure.EXTI_Line = EncoderA_ExtiLine;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EncoderA_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = Button_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    butt.check(button_id, []
    {
        return !GPIO_ReadInputDataBit(Button_Port, Button_Pin);
    });

    butt.check(encoder_button_id, []
    {
        return !GPIO_ReadInputDataBit(EncoderA_Port, EncoderA_Pin);
    });
    //------------------------------
    buzz.init();
    //------------------------------
}
/****************************************************************************/
void app_main()
{
    try
    {
        printf("\rRestart..  ");

        buzz.beep(std::chrono::milliseconds(50));

        butt.press(button_id, []
        {
            printf("\rButton Pressed..");
            buzz.beep(std::chrono::milliseconds(50));
        });

        butt.longpress(button_id, std::chrono::seconds(2), []
        {
            printf("\rButton LongPressed..");
            buzz.beep(std::chrono::milliseconds(200));
        });

        butt.press(encoder_button_id, []
        {
            if(GPIO_ReadInputDataBit(EncoderB_Port, EncoderB_Pin))
            {
                enc_count--;
            }
            else
            {
                enc_count++;
            }
            printf("\rEnc: %d    ", enc_count.load());
            buzz.beep(std::chrono::milliseconds(50));
        });

        //throw std::system_error(0, os_category(), "TEST!!");

        std::thread main_thread([]
        {
            sys::timer buzzerTimer(std::chrono::milliseconds(3000), []
            {
                buzz.beep(std::chrono::milliseconds(10));
                return true;
            });
            buzzerTimer.start();
            sys::chrono::high_resolution_clock::time_point tp1 = sys::chrono::high_resolution_clock::now();
            std::mutex my_mutex;
            for(;;)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                my_mutex.lock();
                printf("\r\nRun: %08X", (uint32_t)(sys::chrono::high_resolution_clock::now() - tp1).count());
                my_mutex.unlock();
            }
        });
    }
    catch(std::exception &e)
    {
        throw;
    }
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

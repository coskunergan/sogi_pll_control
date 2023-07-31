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
#include "mc_spll.h"
#include "gpio_hal.h"

using namespace cmsis;
using namespace control;
using namespace gpio_hal;

SPLL  phase;
button butt;
buzzer buzz;
printf_io my_printf;
uint8_t enc_count{175};

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

/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
extern "C" int _write(int fd, char *pbuffer, int size)
{
    for(int i = 0; i < size; i ++)
    {
        my_printf.io_putchar(*pbuffer++);
    }
    return size;
}
/****************************************************************************/
extern "C" void EXTI0_IRQHandler(void)
{
    butt.isr_handler(button_id);
    EXTI_ClearITPendingBit(EXTI_Line0);
}
/****************************************************************************/
extern "C" void EXTI2_IRQHandler(void)
{
    butt.isr_handler(encoder_button_id);
    EXTI_ClearITPendingBit(EXTI_Line2);
}
/****************************************************************************/
const uint32_t DIFF_DEGREE = 10;
const uint32_t OFFSET_PHASE = 90;
extern "C" void ADC1_IRQHandler(void) // ~155 uSn (3.2KHz)
{
    static uint8_t count = 0;
    static uint16_t adc_value = 0;

    adc_value += ADC_GetConversionValue(ADC1);
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);

    if(++count > 1)
    {
        count = 0;
        adc_value = adc_value / 2;
        phase.transfer_1phase(((float)adc_value)); // ~145 uSn
        adc_value = 0;
        int degree = ((phase.phase() / 3.14159265358979323f) * 180.0f);
        degree += OFFSET_PHASE;
        degree %= 360;
        if((degree > enc_count && degree < (DIFF_DEGREE + enc_count)) ||
                (degree > (enc_count + 180) && degree < (DIFF_DEGREE + enc_count + 180)))
        {
            GPIO_SetBits(GPIOC, GPIO_Pin_7);
        }
        else
        {
            GPIO_ResetBits(GPIOC, GPIO_Pin_7);
        }
    }
}
/****************************************************************************/
void adc_init(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    ADC_DeInit(ADC1);
    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfConversion = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_384Cycles);
    ADC_DelaySelectionConfig(ADC1, ADC_DelayLength_255Cycles);
    ADC_PowerDownCmd(ADC1, ADC_PowerDown_Idle_Delay, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_ADONS) == RESET);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = ADC1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 15;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
    ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    ADC_SoftwareStartConv(ADC1);
}
/****************************************************************************/
void pre_init()
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
    my_printf.init();
    //------------------------------
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    EXTI_InitTypeDef EXTI_InitStructure;

    SYSCFG_EXTILineConfig(EncoderA_ExtiPort, EncoderA_ExtiPin);
    SYSCFG_EXTILineConfig(Button_ExtiPort, Button_ExtiPin);
    RCC_AHBPeriphClockCmd(EncoderA_Clk, ENABLE);
    RCC_AHBPeriphClockCmd(EncoderB_Clk, ENABLE);
    RCC_AHBPeriphClockCmd(Button_Clk, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Pin = Button_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(Button_Port, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;    
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);    

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
    adc_init();
    //------------------------------
    buzz.init();
    //------------------------------
}
/****************************************************************************/
void app_main()
{
    int ignore = 0;

    phase.reset();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    printf("\rRestart..  ");

    buzz.beep(std::chrono::milliseconds(50));

    butt.press(button_id, [&]
    {
        my_printf.turn_off_bl_enable();
        printf("\rButton Pressed..");
        buzz.beep(std::chrono::milliseconds(50));
        ignore = 10;
    });

    butt.longpress(button_id, std::chrono::seconds(2), [&]
    {
        my_printf.turn_off_bl_enable();
        printf("\rButton LongPressed..");
        buzz.beep(std::chrono::milliseconds(200));
        ignore = 10;
    });

    butt.press(encoder_button_id, [&]
    {
        if(GPIO_ReadInputDataBit(EncoderB_Port, EncoderB_Pin))
        {
            if(enc_count > 100)
            {
                enc_count--;
            }
        }
        else
        {
            if(enc_count < 180)
            {
                enc_count++;
            }
        }
        my_printf.turn_off_bl_enable();
        printf("\rEnc: %d    ", enc_count);
        buzz.beep(std::chrono::milliseconds(20));
        ignore = 10;
    });

    //throw std::system_error(0, os_category(), "TEST!!");

    sys::timer buzzerTimer(std::chrono::milliseconds(3000), []
    {
        buzz.beep(std::chrono::milliseconds(10));
        return true;
    });
    buzzerTimer.start();
    int freq = 0;
    float freq_sum = 0;
    for(;;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if(!ignore)
        {
            freq_sum -= freq_sum / 10;
            freq_sum += phase.freq() * phase.freq();
            freq = (freq == 0) ? 1 : freq;
            freq = (freq + ((freq_sum / 10) / freq)) / 2;
            printf("\rF= %d Lock= %d       ", freq, (int)phase.is_lock());
            printf("\r\nPhase = %d         ", (int)((phase.phase() / 3.14159265358979323f) * 180.0f));
        }
        else
        {
            ignore--;
        }
    }
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

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
using namespace device_pll_control;
using namespace device_buzzer;
using namespace device_button;
using namespace gpio_hal;

GpioInput button_pin;
GpioInput encoder_pinA;
GpioInput encoder_pinB;
GpioOutput pulse_pin;
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
            pulse_pin.on();
        }
        else
        {
            pulse_pin.off();
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
    //------------------------------
    my_printf.init();
    //------------------------------
    button_pin.init(GPIOA, GPIO_Pin_0, true);
    button_pin.enablePullup();
    button_pin.enableInterrupt(GpioInput::gpio_int_type_t::fall);

    encoder_pinA.init(GPIOA, GPIO_Pin_2, true);
    encoder_pinA.enablePullup();
    encoder_pinA.enableInterrupt(GpioInput::gpio_int_type_t::fall);

    encoder_pinB.init(GPIOB, GPIO_Pin_4);
    encoder_pinB.enablePullup();

    pulse_pin.init(GPIOC, GPIO_Pin_7);    

    butt.check(button_id, []
    {
        return button_pin.read();
    });

    butt.check(encoder_button_id, []
    {
        return encoder_pinA.read();
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
    cmsis::mutex printf_mutex;

    int ignore = 0;

    buzz.beep(std::chrono::milliseconds(50));

    phase.reset();

    button_pin.setISRHandler([]
    {
        butt.isr_handler(button_id);
    });

    encoder_pinA.setISRHandler([]
    {
        butt.isr_handler(encoder_button_id);
    });      

    printf_mutex.lock();
    printf("\rRestart..  ");
    printf_mutex.unlock();
    my_printf.turn_off_bl_enable();

    std::this_thread::sleep_for(std::chrono::milliseconds(500));    

    butt.press(button_id, [&]
    {
        my_printf.turn_off_bl_enable();
        std::lock_guard<std::mutex> lg(printf_mutex);
        printf("\rButton Pressed..");
        buzz.beep(std::chrono::milliseconds(50));
        ignore = 10;
    });

    butt.longpress(button_id, std::chrono::seconds(2), [&]
    {
        my_printf.turn_off_bl_enable();
        std::lock_guard<std::mutex> lg(printf_mutex);
        printf("\rButton LongPressed..");
        buzz.beep(std::chrono::milliseconds(200));
        ignore = 10;
    });

    butt.press(encoder_button_id, [&]
    {
        if(encoder_pinB.read())
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

    sys::timer buzzerTimer(std::chrono::milliseconds(3000), [&]
    {
        buzz.beep(std::chrono::milliseconds(3));
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
            std::lock_guard<std::mutex> lg(printf_mutex);
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

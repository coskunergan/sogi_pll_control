
#include "printf_io.h"
#include "stdbool.h"

uint8_t ch_count;

/* support printf function, usemicrolib is unnecessary */
#if !defined (__GNUC__)
#if (__ARMCC_VERSION > 6000000)
__asm(".global __use_no_semihosting\n\t");
void _sys_exit(int x)
{
    x = x;
}
/* __use_no_semihosting was requested, but _ttywrch was */
void _ttywrch(int ch)
{
    ch = ch;
}
FILE __stdout;
#else
#ifdef __CC_ARM
#pragma import(__use_no_semihosting)
struct __FILE
{
    int handle;
};
FILE __stdout;
void _sys_exit(int x)
{
    x = x;
}
/* __use_no_semihosting was requested, but _ttywrch was */
void _ttywrch(int ch)
{
    ch = ch;
}
#endif
#endif
#endif

/****************************************************************************/
static void io_pin_init(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin, uint16_t GPIO_clk)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_AHBPeriphClockCmd(GPIO_clk, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_40MHz;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}
/****************************************************************************/
static void io_pins_init(void)
{
    io_pin_init(LCD_RST_PORT, LCD_RST_PIN, LCD_RST_AHB);
    GPIO_ResetBits(LCD_RST_PORT, LCD_RST_PIN);
    io_pin_init(LCD_CLK_PORT, LCD_CLK_PIN, LCD_CLK_AHB);
    io_pin_init(LCD_DTA_PORT, LCD_DTA_PIN, LCD_DTA_AHB);
    io_pin_init(LCD_CMD_PORT, LCD_CMD_PIN, LCD_CMD_AHB);
    io_pin_init(LCD_ENB_PORT, LCD_ENB_PIN, LCD_ENB_AHB);
    io_pin_init(LCD_BL_PORT, LCD_BL_PIN, LCD_BL_AHB);
    GPIO_ResetBits(LCD_ENB_PORT, LCD_ENB_PIN);
    GPIO_ResetBits(LCD_BL_PORT, LCD_BL_PIN);
    GPIO_SetBits(LCD_RST_PORT, LCD_RST_PIN);
}
/****************************************************************************/
static void io_send(uint8_t byte)
{
    for(uint8_t i = 0; i < 8; i++)
    {
        GPIO_ResetBits(LCD_CLK_PORT, LCD_CLK_PIN);
        if((byte & 0x80) == 0x80)
        {
            GPIO_SetBits(LCD_DTA_PORT, LCD_DTA_PIN);
        }
        else
        {
            GPIO_ResetBits(LCD_DTA_PORT, LCD_DTA_PIN);
        }
        byte <<= 1;
        GPIO_SetBits(LCD_CLK_PORT, LCD_CLK_PIN);
    }
}
/****************************************************************************/
extern int __io_putchar(int ch);
void printf_init(void)
{
#if defined (__GNUC__) && !defined (__clang__)
    setvbuf(stdout, NULL, _IONBF, 0);
#endif
    io_pins_init();
    volatile uint8_t i = 0;
    while(--i);
    GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);;
    i = 0;
    do
    {
        io_send(init_params[i++]);
    }
    while(i <= sizeof(init_params));
    io_send(0xAF);
    io_send(0xB0);
    for(uint8_t i = 0; i < 16; i++)
    {
        GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);
        io_send(i);
        io_send(0x10);
        GPIO_SetBits(LCD_CMD_PORT, LCD_CMD_PIN);
        io_send(0x00);
    }
    GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);
    io_send(0xB8);
    for(i = 0; i < 96; i++)
    {
        GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);
        io_send(i & 0xF);
        io_send((i >> 4) | 0x10);
        GPIO_SetBits(LCD_CMD_PORT, LCD_CMD_PIN);
        io_send(0x00);
    }
    GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);
    io_send(0xB0);
    io_send(0x00);
    io_send(0x11);
    GPIO_SetBits(LCD_CMD_PORT, LCD_CMD_PIN);
    ch_count = 0;
}
/****************************************************************************/
#if defined (__GNUC__) && !defined (__clang__)
extern int __io_putchar(int ch);
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif
PUTCHAR_PROTOTYPE
{
    if((ch == '\r') || (ch_count > 25))
    {
        if(ch == '\r')
        {
            ch_count = 0;
        }
        GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);
        io_send(0xB0);
        io_send(0x00);
        io_send(0x11);
        GPIO_SetBits(LCD_CMD_PORT, LCD_CMD_PIN);

    }
    else if(ch == '\n')
    {
        if(ch_count < 14)
        {
            ch_count = 14;
            GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);
            io_send(0xB1);
            io_send(0x00);
            io_send(0x11);
            GPIO_SetBits(LCD_CMD_PORT, LCD_CMD_PIN);
        }
    }
    else
    {
        if(++ch_count == 14)
        {
            io_send(0);
            io_send(0);
            GPIO_ResetBits(LCD_CMD_PORT, LCD_CMD_PIN);
            io_send(0xB1);
            io_send(0x00);
            io_send(0x11);
            GPIO_SetBits(LCD_CMD_PORT, LCD_CMD_PIN);
        }
        uint8_t temp = ch - 0x20;
        for(uint8_t i = 0; i < 6; i++)
        {
            io_send(char_table[temp][i]);
        }
    }
    return ch;
}
/****************************************************************************/
#if defined (__GNUC__) && !defined (__clang__)
int _write(int fd, char *pbuffer, int size)
{
    for(int i = 0; i < size; i ++)
    {
#if defined (__GNUC__) && !defined (__clang__)
        __io_putchar(*pbuffer++);
#else
        fputc(*pbuffer++, (FILE *)NULL);
#endif
    }
    return size;
}
#endif
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

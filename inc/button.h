/*
    Button Lib

    Created on: July 26, 2023

    Author: Coskun ERGAN
*/

#if !defined(__INCLUDE_BUTTON_H__)
#define __INCLUDE_BUTTON_H__

#include "RTE_Components.h"
#include  CMSIS_device_header
#include <chrono>
#include "mutex.h"
#include "timer.h"
#include "threadflag.h"
#include <functional>

const size_t max_button_num = 3; // max val => 31
const std::chrono::milliseconds m_button_check_msec = std::chrono::milliseconds(100);

class button
{
public:
    void check(size_t id, std::function<bool()> &&handler)
    {
        m_button_check_handler[id] = std::move(handler);
    }

    void press(size_t id, std::function<void()> &&handler)
    {
        if(!m_thread_ptr)
        {
            std::thread button_thread([&]
            {
                cmsis::this_thread::flags th_flag;
                size_t id;
                uint32_t mask;
                uint32_t m_mask = 0;
                uint32_t temp_mask;
                cmsis::this_thread::flags::status status;
                std::chrono::system_clock::time_point tp;
                std::chrono::system_clock::time_point now;
                for(;;)
                {
                    if(m_mask)
                    {
                        if(tp <= now)
                        {
                            tp = now + std::chrono::milliseconds(1);
                        }
                        status = th_flag.wait_for(0x7FFFFFFF, std::chrono::duration_cast<std::chrono::milliseconds>((tp - now)), mask);
                        th_flag.clear();
                        now = std::chrono::system_clock::now();
                    }
                    else
                    {
                        mask = th_flag.wait(0x7FFFFFFF);
                        th_flag.clear();
                        now = std::chrono::system_clock::now();
                        tp = now + std::chrono::milliseconds(m_button_check_msec);
                        status = decltype(status)::no_timeout;
                    }
                    if(status == decltype(status)::no_timeout)
                    {
                        temp_mask = m_mask | mask;
                        id = 0;
                        while(temp_mask)
                        {
                            while((temp_mask & 1) == 0)// find set bit
                            {
                                id++;
                                temp_mask >>= 1;
                            }
                            if((m_mask & (1 << id)) == 0)// first press
                            {
                                m_long_press_tp[id] = std::chrono::milliseconds(m_long_press_msec[id]) + now;
                                m_button_press_handler[id]();
                            }
                            id++;
                            temp_mask >>= 1;
                        }
                        m_mask |= mask;
                    }
                    else //if(status == decltype(status)::timeout)
                    {
                        tp = now + std::chrono::milliseconds(m_button_check_msec);
                        temp_mask = m_mask;
                        id = 0;
                        while(temp_mask)
                        {
                            while((temp_mask & 1) == 0)// find set bit
                            {
                                id++;
                                temp_mask >>= 1;
                            }
                            if(m_button_check_handler[id]())
                            {
                                if(m_button_longpress_handler[id])
                                {
                                    if(now > m_long_press_tp[id])
                                    {
                                        m_button_longpress_handler[id]();
                                    }
                                }
                            }
                            else
                            {
                                m_mask &= ~(1UL << id);
                            }
                            id++;
                            temp_mask >>= 1;
                        }
                    }
                }
            });
            m_thread_ptr = std::make_unique<sys::thread>(std::move(button_thread));
        }
        m_button_press_handler[id] = std::move(handler);
    }

    void longpress(size_t id, std::chrono::milliseconds(long_press_msec), std::function<void()> &&handler)
    {
        m_long_press_msec[id] = long_press_msec;
        m_button_longpress_handler[id] = std::move(handler);
    }

    void isr_handler(size_t id)
    {
        m_th_flag.set(*m_thread_ptr, (1 << id));
    }

private:
    std::unique_ptr<sys::timer> m_tim_ptr;
    std::chrono::milliseconds m_long_press_msec[max_button_num];
    std::function<bool()> m_button_check_handler[max_button_num];
    std::function<void()> m_button_press_handler[max_button_num];
    std::function<void()> m_button_longpress_handler[max_button_num];
    std::chrono::system_clock::time_point m_long_press_tp[max_button_num];
    std::unique_ptr<sys::thread> m_thread_ptr;
    cmsis::thread_flags m_th_flag;

};

#endif

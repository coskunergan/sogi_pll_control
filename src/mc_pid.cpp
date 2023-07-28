/*
    PID Library

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/
#include "mc_pid.h"

using namespace control;

void PID::reset() noexcept
{
    i_sum = 0;
    sat_err = 0;
}

value_t PID::p_transfer(value_t e) noexcept
{
    value_t m = param.kp * e;
    value_t out
        = m > param.i_max ? param.i_max
          : m < param.i_min ? param.i_min
          : m;
    return out;
}

value_t PID::pi_transfer(value_t e) noexcept
{
    value_t sat = param.kp * e + i_sum;
    // U(s) = kp * E(s) + ki * E(s) / s
    value_t out
        = sat > param.i_max ? param.i_max
          : sat < param.i_min ? param.i_min
          : sat;

    sat_err = out - sat;

    i_sum += param.ki * e + param.kc * sat_err;

    if(i_sum > param.i_max)
    {
        i_sum = param.i_max;
    }
    else if(i_sum < param.i_min)
    {
        i_sum = param.i_min;
    }
    return out;
}

value_t PID::pd_transfer(value_t e) noexcept
{
    return 0;
}

value_t PID::pid_transfer(value_t e) noexcept
{
    return 0;
}

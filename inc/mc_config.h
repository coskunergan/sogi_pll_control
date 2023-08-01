/*
    Control Config

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/
#pragma once
#include <float.h>
namespace device_pll_control
{
    using value_t = float;

    constexpr value_t T = 1 / 3200.0f;

    template<typename T>

    struct constant_value;

    template<>

    struct constant_value<value_t>
    {
        static constexpr value_t ZERO = 0;

        static constexpr value_t PI = 3.14159265358979323f;

        static constexpr value_t HALF_PI = PI / 2;

        static constexpr value_t TAU = PI * 2;

        static constexpr value_t MAX = 4096;

        static constexpr value_t MIN = 0;
    };

}

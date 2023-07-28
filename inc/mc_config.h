/*
    Control Config

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/
#if !defined(__INCLUDE_CONTROL_MC_CONFIG_H__)
#define __INCLUDE_CONTROL_MC_CONFIG_H__
#include <float.h>
namespace control
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
#endif

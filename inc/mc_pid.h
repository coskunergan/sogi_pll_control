/*
    PID Lib

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/

#if !defined(__INCLUDE_CONTROL_MC_PID_H__)
#define __INCLUDE_CONTROL_MC_PID_H__
#include "mc_config.h"
namespace control
{
    class PID final
    {
    public:
        struct Factors
        {
            value_t kp;

            value_t ki;

            value_t kd;

            value_t kg;

            value_t kc;

            value_t i_min;

            value_t i_max;
        };

        PID() = default;
        ~PID() = default;

        PID(const PID &) = delete;
        PID &operator=(const PID &) = delete;

        PID(PID &&) = delete;
        PID &operator=(PID &&) = delete;

        Factors param = {0};

        void reset() noexcept;

        value_t p_transfer(value_t e) noexcept;

        value_t pi_transfer(value_t e) noexcept;

        value_t pd_transfer(value_t e) noexcept;

        value_t pid_transfer(value_t e) noexcept;

    private:
        value_t sat_err = 0;

        value_t i_sum = 0;
    };
}
#endif

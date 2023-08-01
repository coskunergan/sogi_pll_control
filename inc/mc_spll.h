/*
    SOGI PLL Library

    Created on: March 13, 2023

    Author: Coskun ERGAN
*/
#pragma once
#include <tuple>
#include "mc_pid.h"
#include <stddef.h>
#include <stdint.h>
#include "mc_config.h"
namespace device_pll_control
{
    class SPLL final
    {
    public:

        static constexpr value_t Ti = T;

        static constexpr value_t TARGET_FREQ = 50;

        static constexpr size_t N_SAMPLE = static_cast<size_t>((1 / Ti) / TARGET_FREQ);

        SPLL();
        ~SPLL() = default;

        SPLL(SPLL &&) = delete;
        SPLL &operator=(SPLL &&) = delete;

        SPLL(const SPLL &) = delete;
        SPLL &operator=(const SPLL &) = delete;

        void reset();

        void transfer_1phase(value_t val);

        bool is_lock(value_t th = 1e-2f) const;

        inline value_t freq() const noexcept
        {
            return omega / constant_value<value_t>::TAU;
        }

        inline value_t phase() const noexcept
        {
            return cur_phase;
        }
    private:

        PID pid;

        bool launch_loop;

        uint16_t sample_index;

        value_t omega;

        value_t cur_phase;

        value_t auto_offset_min;

        value_t auto_offset_max;

        value_t sogi_s1;

        value_t sogi_s2;

        value_t last_error;

        value_t auto_offset(value_t inp);
    };
}

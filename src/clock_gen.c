#include "clock_gen.h"

void _cg_rise_fall(volatile struct cg_clock* clk)
{
    if (clk->_pos == 0) {
        if (clk->_div_cnt == 0) {
            clk->state = 1;
        }
    } else if (clk->_mod_pos == 0) {
        if (clk->_mult_cnt < clk->mult) {
            clk->state = 1;
        }
    } else if (clk->_mod_pos == clk->duty) {
        clk->state = 0;
    }
}

void _cg_advance(volatile struct cg_clock* clk)
{
    if (++(clk->_pos) == clk->period) {
        clk->_pos = 0;
        ++(clk->_div_cnt);
    }

    if (++(clk->_mod_pos) == clk->_mod_period + (clk->_comp_mask & 1)) {
        clk->_mod_pos = 0;
        ++(clk->_mult_cnt);
        (clk->_comp_mask) >>= 1;
    }
}

void _cg_reset(volatile struct cg_clock* clk)
{
    clk->_mod_pos = 0;
    clk->_mult_cnt = 0;
    // BUG
    if (clk->_div_cnt >= clk->div) {
        clk->_div_cnt = 0;
    }
}

void _cg_get(volatile struct cg_clock* clk)
{
    clk->period = clk->_pending.period;
    clk->div = clk->_pending.div;
    clk->mult = clk->_pending.mult;
    clk->_mod_period = clk->_pending._mod_period;
    clk->_comp_mask = clk->_pending._comp_mask;
}

void cg_tick(volatile struct cg_clock* clk)
{
    _cg_rise_fall(clk);
    _cg_advance(clk);
    if (clk->_pos == 0) {
        _cg_get(clk);
        _cg_reset(clk);
    }
}

void cg_set(volatile struct cg_clock* clk, uint16_t period, uint8_t div,
            uint8_t mult)
{
    clk->_pending.period = period;
    clk->_pending.div = div;
    clk->_pending.mult = mult;
    clk->_pending._mod_period = (period * div) / mult;

    uint16_t comp_ticks = (period * div) % mult;
    clk->_pending._comp_mask = UINT32_MAX >> (32 - comp_ticks);
}

void cg_set_period(volatile struct cg_clock* clk, uint16_t period)
{
    clk->_pending.period = period;
    clk->_pending._mod_period = period;
}

void cg_init(struct cg_clock* clk, uint16_t period, uint8_t duty, uint8_t div,
             uint8_t mult)
{
    clk->duty = duty;
    cg_set(clk, period, div, mult);
    _cg_get(clk);
}

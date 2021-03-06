#ifndef CLOCK_GEN_H
#define CLOCK_GEN_H

#include <stdint.h>

struct cg_clock {
    /* one beat in ticks */
    uint16_t period;
    /* trigger length in ticks */
    uint8_t duty;
    /* clock divider (makes long notes) */
    uint8_t div;
    /* clock multiplier (makes short notes) */
    uint8_t mult;
    /* current binary state of trigger */
    uint8_t state;

    /* current clock position in ticks */
    uint16_t _pos;
    /* one sub beat of multiplied clock in ticks */
    uint16_t _mod_period;
    /* current multiplied sub clock position in ticks */
    uint16_t _mod_pos;
    /* bit mask to compensate multiplied clock triggers timing
    where each bit represents added tick to the next sub beat
    */
    uint32_t _comp_mask;
    /* elapsed multiplied clock sub beats */
    uint8_t _mult_cnt;
    /* elapsed divided clock sub beats */
    uint8_t _div_cnt;

    /* scheduled new state */
    struct {
        uint32_t _comp_mask;
        uint16_t _mod_period;
        uint16_t period;
        uint8_t div;
        uint8_t mult;
    } _pending;
};

/* Sets initial state. */
void cg_init(struct cg_clock* clk, uint16_t period, uint8_t duty, uint8_t div,
             uint8_t mult);

/* Performs one work cycle. */
void cg_tick(volatile struct cg_clock* clk);

/* Schedules state change. */
void cg_set(volatile struct cg_clock* clk, uint16_t period, uint8_t div,
            uint8_t mult);

/* Schedules period change. */
void cg_set_period(volatile struct cg_clock* clk, uint16_t period);

#endif

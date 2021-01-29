#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "device.h"
#include "macros.h"
#include "adc_freerunner.h"

typedef uint16_t MasterClockTick;
typedef uint16_t SubClockTick;

#define TIMER_FREQ 125000L
#define TIMER_PRESCALER 2

const uint8_t TIMER_COMP_TICKS = F_CPU / TIMER_PRESCALER / TIMER_FREQ - 1;

const SubClockTick B_MULTIPLIERS[] = { 48*8 , 48*6 , 48*4, 48*3, 48*2, 48*1, 48/2, 48/3, 48/4, 48/6 , 48/8  };
const SubClockTick C_MULTIPLIERS[] = { 48*24, 48*12, 48*8, 48*4, 48*2, 48*1, 48/2, 48/4, 48/8, 48/12, 48/24 };

void reset();

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min + 1) / (in_max - in_min + 1) + out_min;
}

template<uint8_t TYPE>
class ClockGenerator {
private:
public:
    volatile SubClockTick period;
    volatile SubClockTick duty;
    volatile SubClockTick pos;
  
    void setPeriod(SubClockTick ticks) {
        duty = ticks>>1;
        period = ticks-1;
    }
    
    void resetPhase() {
        pos = -1;
        on();
    }

    void clock(){
        if (++pos == duty) {
            off();
        }
        
        if (pos > period) {
            pos = 0;
            on();
        }
    }

    void toggle() {
        if (isOff()) {
            on();
        }
        else {
            off();
        }
    }

    void on();

    void off();

    bool isOff();
};

enum { CLOCK_TYPE_A, CLOCK_TYPE_B, CLOCK_TYPE_C };

// clock A
template<> void ClockGenerator<CLOCK_TYPE_A>::on() {
    GENERATOR_OUTA_PORT &= ~_BV(GENERATOR_OUTA_PIN);
    reset();
}

template<> void ClockGenerator<CLOCK_TYPE_A>::off() {
    GENERATOR_OUTA_PORT |= _BV(GENERATOR_OUTA_PIN);
}

template<> bool ClockGenerator<CLOCK_TYPE_A>::isOff() {
    return GENERATOR_OUTA_PORT & _BV(GENERATOR_OUTA_PIN);
}

// clock B
template<> void ClockGenerator<CLOCK_TYPE_B>::on() {
    GENERATOR_OUTB_PORT &= ~_BV(GENERATOR_OUTB_PIN);
}

template<> void ClockGenerator<CLOCK_TYPE_B>::off() {
    GENERATOR_OUTB_PORT |= _BV(GENERATOR_OUTB_PIN);
}

template<> bool ClockGenerator<CLOCK_TYPE_B>::isOff() {
    return GENERATOR_OUTB_PORT & _BV(GENERATOR_OUTB_PIN);
}

// clock C
template<> void ClockGenerator<CLOCK_TYPE_C>::on() {
    GENERATOR_OUTC_PORT &= ~_BV(GENERATOR_OUTC_PIN);
}

template<> void ClockGenerator<CLOCK_TYPE_C>::off() {
    GENERATOR_OUTC_PORT |= _BV(GENERATOR_OUTC_PIN);
}

template<> bool ClockGenerator<CLOCK_TYPE_C>::isOff() {
    return GENERATOR_OUTC_PORT & _BV(GENERATOR_OUTC_PIN);
}

ClockGenerator<CLOCK_TYPE_A> clockA;
ClockGenerator<CLOCK_TYPE_B> clockB;
ClockGenerator<CLOCK_TYPE_C> clockC;

class MasterClock {
private:
public:
    volatile bool resetB = false;
    volatile bool resetC = false;
    volatile MasterClockTick period;
    MasterClockTick pos;
  
    void setPeriod(MasterClockTick ticks) {
        period = ticks;
    }

    void clock() {
        if (++pos > period) {
            pos = 0;
            clockA.clock();
            clockB.clock();
            clockC.clock();
        }
    }
    
    void reset() {
        if (resetB) {
            clockB.resetPhase();
            resetB = false;
        }

        if (resetC) {
            clockC.resetPhase();
            resetC = false;
        }
    }
};

MasterClock master;

void reset(){
    master.reset();
}

void setup_output() {
    GENERATOR_OUTA_DDR |= _BV(GENERATOR_OUTA_PIN);
    GENERATOR_OUTB_DDR |= _BV(GENERATOR_OUTB_PIN);
    GENERATOR_OUTC_DDR |= _BV(GENERATOR_OUTC_PIN);
}

void setup_timer() {
    // ctc mode
    TCCR1 |= _BV(CTC1);
    // prescaler 2
    TCCR1 |= _BV(CS11);
    // reset timer
    TCNT1 = 0;
    // compare value
    OCR1C = TIMER_COMP_TICKS;
    // enable interrupt
    TIMSK |= _BV(OCIE1A);
}

void setup() {
    cli();
    setup_output();
    setup_timer();
    setup_adc();
    sei();
}

SubClockTick mulB = 5;
SubClockTick mulC = 5;

void loop() {
    SubClockTick b = map(getAnalogValue(GENERATOR_RATE_B_CONTROL), 0, ADC_VALUE_RANGE, 0, 10);
    if (b != mulB) {
        mulB = b;
        master.resetB = true;
    }

    SubClockTick c = map(getAnalogValue(GENERATOR_RATE_C_CONTROL), 0, ADC_VALUE_RANGE, 0, 10);
    if (c != mulC) {
        mulC = c;
        master.resetC = true;
    }
    
    // desired ticks count derived from (TIMER_FREQ / CLOCK_PRESCALER_48 / (BPM / 60 * 4))
    MasterClockTick a = map(getReversedAnalogValue(GENERATOR_RATE_A_CONTROL), 0, ADC_VALUE_RANGE, 176, 968);
    
    master.setPeriod(a);
    
    clockA.setPeriod(48);
    clockB.setPeriod(B_MULTIPLIERS[b]);
    clockC.setPeriod(C_MULTIPLIERS[c]);
}

#define GENERATOR_TIMER_COMP_VECTOR TIMER1_COMPA_vect
ISR(GENERATOR_TIMER_COMP_VECTOR) {
    master.clock();
}
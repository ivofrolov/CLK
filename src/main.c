#include <avr/interrupt.h>
#include <avr/io.h>
#include <util/atomic.h>
#include "device.h"
#include "clock_gen.h"

uint16_t volatile pots_values[ADC_CHANNELS];

struct cg_clock clock_a;
struct cg_clock clock_b;
struct cg_clock clock_c;

/* Configures clocks instances initial state. */
void init_clocks()
{
    cg_init(&clock_a, 500, TRIGGER_DUTY, 1, 1);
    cg_init(&clock_b, 500, TRIGGER_DUTY, 1, 1);
    cg_init(&clock_c, 500, TRIGGER_DUTY, 1, 1);
}

/* Configures trigger output pins. */
void init_output()
{
    PULSERS_DDR |= PULSERS_PINS_MASK;  // configures pins as output
}

/* Configures Timer0 to generate interrupt in CTC mode.

Maximum frequency range that is sufficient is [60:240] bpm / 60 × 24 = [24:96] Hz.
Usual trigger length is [2:10] ms = [200:1000] Hz (of square wave with 50% duty cycle).
So let's use 1000 Hz (1 ms period) as target timer frequency.
*/
void init_timer()
{
    TCCR0A |= _BV(WGM01);  // configures ctc mode
    TCCR0B |= _BV(CS01) | _BV(CS00);  // sets F_CPU/64 prescaler
    OCR0A = 124;  // sets compare value
    TIMSK |= _BV(OCIE0A);  // enables interrupt on compare match
}

/* Moves the clocks one tick and writes their states to output. */
ISR(TIMER0_COMPA_vect)
{
    cg_tick(&clock_a);
    cg_tick(&clock_b);
    cg_tick(&clock_c);

    uint8_t states = (clock_a.state << PULSER_A_PIN)
                     | (clock_b.state << PULSER_B_PIN)
                     | (clock_c.state << PULSER_C_PIN);
    // writes pin states to output register in one pass
    PULSERS_PORT = (PULSERS_PORT & ~PULSERS_PINS_MASK) | states;
}

/* Configures ADC to generate interrupt in Free Running mode. */
void init_adc()
{
    ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);  // sets F_CPU/128 prescaler
    ADCSRA |= _BV(ADATE);  // enables auto trigger
    ADCSRA |= _BV(ADEN) | _BV(ADSC);  // enables ADC and starts conversion
    ADCSRA |= _BV(ADIE);  // enables ADC interrupt
}

/* Reads pots values with a bit of oversampling. */
ISR(ADC_vect)
{
    static uint8_t prev_channel;
    static uint8_t counter;
    static uint16_t buf[ADC_CHANNELS];

    uint8_t curr_channel = ADMUX & ADC_CHANNELS_MASK;
    buf[prev_channel] += ADCL | (ADCH << 8);
    prev_channel = curr_channel;

    if (++curr_channel == ADC_CHANNELS) {
        curr_channel = 0;
        if (++counter == ADC_OVERSAMPLING) {
            counter = 0;
            for (uint8_t i = 0; i < ADC_CHANNELS; ++i) {
                pots_values[i] = buf[i];
                buf[i] = 0;
            }
        }
    }

    ADMUX = (ADMUX & ~ADC_CHANNELS_MASK) | curr_channel;
}

/* Initializes hardware and software things. */
void setup()
{
    init_clocks();

    cli();
    init_output();
    init_timer();
    init_adc();
    sei();
}

/* Sets clocks parameters if pots values were changed. */
void loop()
{
    static uint16_t prev_period;
    static uint16_t prev_mod_b;
    static uint16_t prev_mod_c;
    static uint8_t params_changed = 0;

    // TODO: make macros to figure out these magical ⌄⌄⌄⌄⌄⌄⌄ numbers
    uint16_t period = pots_values[PARAM_A_CHANNEL] * 11 / 60 + 250;
    if (period != prev_period) {
        params_changed = 1;
        prev_period = period;
    }

    uint16_t mod_b = pots_values[PARAM_B_CHANNEL] * 11 / ADC_VALUE_RANGE;
    if (mod_b != prev_mod_b) {
        params_changed = 1;
        prev_mod_b = mod_b;
    }

    uint16_t mod_c = pots_values[PARAM_C_CHANNEL] * 11 / ADC_VALUE_RANGE;
    if (mod_c != prev_mod_c) {
        params_changed = 1;
        prev_mod_c = mod_c;
    }

    if (params_changed) {
        ATOMIC_BLOCK(ATOMIC_FORCEON) {
            cg_set_period(&clock_a, period);
            cg_set(&clock_b, period,
                   PARAM_B_MODIFIERS[mod_b].div, PARAM_B_MODIFIERS[mod_b].mult);
            cg_set(&clock_c, period,
                   PARAM_C_MODIFIERS[mod_c].div, PARAM_C_MODIFIERS[mod_c].mult);
        }
        params_changed = 0;
    }
}

int main()
{
    setup();
    for (;;)
        loop();
}

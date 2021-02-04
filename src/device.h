#define PULSERS_DDR DDRB
#define PULSERS_PORT PORTB
#define PULSER_A_PIN PORTB3
#define PULSER_B_PIN PORTB1
#define PULSER_C_PIN PORTB0

const uint8_t PULSERS_PINS_MASK =
    _BV(PULSER_A_PIN) | _BV(PULSER_B_PIN) | _BV(PULSER_C_PIN);

#define PARAM_A_CHANNEL 2
#define PARAM_B_CHANNEL 1
#define PARAM_C_CHANNEL 0

#define ADC_CHANNELS 3
const uint8_t ADC_CHANNELS_MASK = _BV(MUX2) | _BV(MUX1) | _BV(MUX0);

#define ADC_OVERSAMPLING 4
const uint16_t ADC_VALUE_RANGE = 1024 * ADC_OVERSAMPLING;

#define MOD_COUNT 11
#define MOD_IDLE 5
struct _mod {
    uint8_t div;
    uint8_t mult;
};
const struct _mod PARAM_B_MODIFIERS[MOD_COUNT] = {
    {8, 1}, {6, 1}, {4, 1}, {3, 1}, {2, 1}, {1, 1}, {1, 2}, {1, 3}, {1, 4}, {1, 6}, {1, 8},
};
const struct _mod PARAM_C_MODIFIERS[MOD_COUNT] = {
    {24, 1}, {12, 1}, {8, 1}, {4, 1}, {2, 1}, {1, 1}, {1, 2}, {1, 4}, {1, 8}, {1, 12}, {1, 24},
};

#define TRIGGER_DUTY 3  // ms

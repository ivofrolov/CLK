#ifndef _ANALOGREADER_H_
#define _ANALOGREADER_H_

#include <inttypes.h>

#define ADC_CHANNELS 3
#define ADC_OVERSAMPLING 4
#define ADC_VALUE_RANGE (1023 * ADC_OVERSAMPLING)

extern uint16_t volatile adc_values[ADC_CHANNELS];

void setup_adc();

#define getAnalogValue(i) (ADC_VALUE_RANGE - adc_values[i])
#define getReversedAnalogValue(i) adc_values[i]

#endif /* _ANALOGREADER_H_ */
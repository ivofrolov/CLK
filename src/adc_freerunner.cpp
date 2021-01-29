#include <avr/interrupt.h> 
#include "adc_freerunner.h"

uint16_t volatile adc_values[ADC_CHANNELS];

void setup_adc() {
    // prescaler 64 - 125kHz sample rate @ 8MHz
    ADCSRA |= _BV(ADPS2) | _BV(ADPS1);
    // auto trigger mode
    ADCSRA |= _BV(ADATE);
    // enable ADC
    ADCSRA |= _BV(ADEN);
    // start A2D conversion
    ADCSRA |= _BV(ADSC);
    // enable ADC interrupt
    ADCSRA |= _BV(ADIE);
}

ISR(ADC_vect) {
    static uint8_t oldchan;
    static uint8_t counter;
    static uint16_t adc_buffer[ADC_CHANNELS];
    
    uint8_t curchan = ADMUX & 7;
    adc_buffer[oldchan] += ADCL | (ADCH << 8);
    oldchan = curchan;
    
    if(++curchan == ADC_CHANNELS) {
        curchan = 0;
        if(++counter == ADC_OVERSAMPLING) {
            counter = 0;
            for(uint8_t i=0; i<ADC_CHANNELS; ++i) {
                adc_values[i] = adc_buffer[i];
                adc_buffer[i] = 0;
            }
        }
    }
    
    ADMUX = (ADMUX & ~7) | curchan;
}
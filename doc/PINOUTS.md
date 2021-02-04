```
╔═══════════════════════════╗
║      ATTINY85 PINOUT      ║
╚═══════════════════════════╝

                                  ┌○────┐
       (PCINT5/RESET/ADC0/dW) PB5 □1   8□ VCC
(PCINT3/XTAL1/CLKI/OC1B/ADC3) PB3 □2   7□ PB2 (SCK/USCK/SCL/ADC1/T0/INT0/PCINT2)
(PCINT4/XTAL2/CLKO/OC1B/ADC2) PB4 □3   6□ PB1 (MISO/DO/AIN1/OC0B/OC1A/PCINT1)
                              GND □4   5□ PB0 (MOSI/DI/SDA/AIN0/OC0A/OC1A/AREF/PCINT0)
                                  └─────┘


╔═══════════════════════════╗
║      AVR ISP PINOUTS      ║
╚═══════════════════════════╝

  10 pin IDC      6 pin IDC

      1 2            1 2
     ┏═══┓          ┏═══┓
 MOSI┃■ ●┃VCC   MISO┃■ ●┃VCC
   NC┃● ●┃GND    SCK ● ●┃MOSI
  RST ● ●┃GND    RST┃● ●┃GND
  SCK┃● ●┃GND       ┗━━━┛
 MISO┃● ●┃GND        5 6
     ┗━━━┛
      9 10


╔═══════════════════════════╗
║    EURORACK POWER BUS     ║
╚═══════════════════════════╝

 ┏═══┓<---- red stripe
 ┃■ ■┃-12V
 ┃● ●┃GND
 ┃● ●┃GND
  ● ●┃GND
  ● ●┃+12V
 ┃● ●┃+5V
 ┃● ●┃CV
 ┃● ●┃GATE
 ┗━━━┛

```
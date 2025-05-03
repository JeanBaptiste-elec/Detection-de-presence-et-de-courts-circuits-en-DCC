/*

  Détection de présence et courts-circuits pour ATtiny44 (84)

  © Christophe BOBILLE 04/24 pour locoduino (www.locoduino.org)
  v 0.5

  Datasheet :
  https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-7701_Automotive-Microcontrollers-ATtiny24-44-84_Datasheet.pdf

  Getting Started with ADC on ATTiny
  https://ww1.microchip.com/downloads/en/Appnotes/TB3209-Getting-Started-with-ADC-90003209A.pdf

  PA0 - PA1 / RX - TX

*/

#include <avr/io.h>


// Variables
uint32_t tempoCC = 5000UL;                             // Tempo réarmement suite à court-circuit
const uint16_t tabSeuilOcc[] = { 30, 40, 50, 60 };     // Seuil pour l'occupation
const uint16_t tabSeuilCc[] = { 250, 350, 450, 550 };  // Seuil pour le court-circuit
uint16_t seuilOcc;
uint16_t seuilCc;

// Etats
byte etatSwitchOcc;  // Etat des bits pour switch occupation
byte etatSwitchCc;   // Etat des bits pour switch court-circuit


uint16_t get_ADC_sample(void) {
  ADCSRA |= (1 << ADSC);  // start ADC measurement
  while (ADCSRA & (1 << ADSC))
    ;
  // return the ADC value
  return (ADCL | (ADCH << 8));
}



void setup() {

  DDRA |= (1 << PA1);   // Signal occupation
  DDRA &= ~(1 << PA4);  // Switch 0
  DDRA &= ~(1 << PA5);  // Switch 1
  DDRA &= ~(1 << PA6);  // Switch 2
  DDRA &= ~(1 << PA7);  // Switch 3
  DDRA |= (1 << PA2);   // Signal cc
  DDRB |= (1 << PB2);   // Relais

  etatSwitchOcc = (PINA & 0x30) >> 4;
  etatSwitchCc = (PINA & 0xC0) >> 6;

  seuilOcc = tabSeuilOcc[etatSwitchOcc];
  seuilCc = tabSeuilCc[etatSwitchCc];


  ADMUX =
    (0 << ADLAR) |  // do not left shift result (for 10-bit values)
    (0 << REFS1) |  // Sets ref. voltage to internal 1.1V
    (0 << REFS0) |  // Sets ref. voltage to internal 1.1V
    (0 << MUX3) |   // use ADC3 for input (PB4), MUX bit 3
    (0 << MUX2) |   // use ADC3 for input (PB4), MUX bit 2
    (1 << MUX1) |   // use ADC3 for input (PB4), MUX bit 1
    (1 << MUX0);    // use ADC3 for input (PB4), MUX bit 0

  ADCSRA =
    (1 << ADEN) |   // Enable ADC
    (1 << ADPS2) |  // set prescaler to 64, bit 2
    (1 << ADPS1) |  // set prescaler to 64, bit 1
    (1 << ADPS0);   // set prescaler to 64, bit 0


  PORTB |= (1 << PB2);   // Relais
  PORTA &= ~(1 << PA1);  // Signal occupation
  PORTA &= ~(1 << PA2);  // Signal cc
}


void loop() {

  uint16_t sample = 0;
  for (byte i = 0; i < 4; i++)  // 4 lectures pour lisser le résultat
    sample += get_ADC_sample();
  sample = sample >> 2;


if (sample < seuilOcc) {
    PORTA &= ~(1 << PA1);  // Signal occupation
} else {
    PORTA |= (1 << PA1);   // Signal occupation
}

  if (sample >= seuilCc) {
    PORTB &= ~(1 << PB2);  // Relais
    PORTB |= (1 << PA2);   // Signal cc
    delay(tempoCC);
    PORTB |= (1 << PB2);   // Relais
    PORTB &= ~(1 << PA2);  // Signal cc
  }
  delay(1);
}

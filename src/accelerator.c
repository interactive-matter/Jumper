/*
 * accelerator.c
 *
 *  http://interactive-matter.org/
 *
 *  This file is part of Jumper.
 *
 *  Jumper is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Jumper is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *  You should have received a copy of the GNU General Public License
 *  along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 *  Created on: 14.02.2010
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define ACCELERATOR_CHANNEL 2
#define ACCELERATOR_PIN _BV(4)
#define ACCELERATOR_VOLTAGE_REFERENCE 0 // _BV(6) would be internal

void
accelerator_start_measurement();

void accelerator_init(void) {
  //accelerator pin is input and no high Z
  DDRB &= ~(ACCELERATOR_PIN);
  PORTB &= ~(ACCELERATOR_PIN);
  accelerator_start_measurement();
}


void
accelerator_start_measurement()
{

  //select the channel & voltage reference
  ADMUX = ACCELERATOR_CHANNEL | ACCELERATOR_VOLTAGE_REFERENCE;

  //enable ADC, slowest conversion
  ADCSRA = _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

  ADCSRA |= _BV(ADEN);//enable ADC with dummy conversion

  //wait till the voltage reference is up
  _delay_us(70);

  //do one dummy measurement
  ADCSRA |= _BV(ADSC); // eine ADC-Wandlung

  while (ADCSRA & _BV(ADSC))
    {
      ; // auf Abschluss der Konvertierung warten
    }
  uint16_t result = ADCW;
  result = 0;
}

uint8_t
accelerator_read()
{

  uint16_t result = 0;

  int loop = 4;
  do
    {
      ADCSRA |= _BV(ADSC); // eine ADC-Wandlung
      while (ADCSRA & _BV(ADSC))
        {
          ; // auf Abschluss der Konvertierung warten
        }
      result += ADCW;
      loop--;
    }
  while (loop);

  result = (result >> 8); //divide result by 4 and shift 2 bits

  return result;
}

void
accelerator_finish_measurement(void)
{

  ADCSRA &= (uint8_t) ~(_BV(ADEN)); // ADC deaktivieren (2)
}

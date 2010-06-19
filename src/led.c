/*
 * led.c
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

#include <avr/interrupt.h>
#include <util/delay.h>

#include "accelerator.h"

#define LED_PIN _BV(3)

volatile uint8_t led_calculate_state = 0;
volatile uint8_t led_value = 0;

void
led_init(void)
{

  DDRA |= LED_PIN;
}

void
led_start() {
    led_on();
  _delay_ms(500);
    led_off();

  //we start with a dark led
  OCR0B = 0;

  //start the LED timer
  //FAST PWM set at top, clear at b
  TCCR0A = _BV( COM0B1 ) | _BV(COM0B0) | _BV(WGM01) | _BV(WGM00);
  TCCR0B = _BV(CS01) | _BV(CS00) | _BV(CS00);
  //we calculate new values after the timer has overflown
  //& update the value after we reach top
  TIMSK0 = _BV(TOIE0);

}

void led_on()
{
    //flashit!
    PORTA |= LED_PIN;
}

void led_off()
{
    PORTA &= ~(LED_PIN);
}

int16_t led_activity = 0;
uint8_t accel_pre_read = 0;
void
led_calculate(void)
{
  if (led_calculate_state)
    {
      int8_t accel_read=accelerator_read();
      uint8_t accel = 0;
      if (accel_read > accel_pre_read) {
        accel = accel_read - accel_pre_read;
      } else {
        accel = accel_pre_read - accel_read;
      }
      //poor mens exp function - sorta
      if (accel & _BV(1)) {
        accel = accel << 1;
      } else if (accel & _BV(2)) {
        accel = accel << 2;
      } else if (accel & _BV(3)) {
        accel = accel << 3;
      } else if (accel & _BV(4)) {
        accel = accel << 4;
      } else if (accel & _BV(5)) {
        accel = accel << 5;
      } else if (accel & _BV(6)) {
        accel = accel << 6;
      } else if (accel & _BV(7)) {
        accel = accel << 7;
      }
      led_activity = ((led_activity + (accel << 8)) - 8);
      //ensure that it stays well in limits
      if (led_activity<0) {
        led_activity=0;
      } else if (led_activity > 0x3FF) {
        led_activity=0x3FF;
      }
      led_value = led_activity >> 6;
      accel_pre_read = accel_read;
      led_calculate_state = 0;
    }
}

ISR(TIM0_OVF_vect)
{
  GTCCR |= _BV(TSM);
  OCR0B = 255 - led_value;
  GTCCR &= ~(_BV(TSM));
  led_calculate_state = 1;
}

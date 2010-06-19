/*
 * main.c
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

//we redefine the F_CPU - since there seems to be a problem
//with avrclipse
#define F_CPU 8000000u;

#include <avr/common.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/power.h>

#include "accelerator.h"
#include "led.h"

#include "accelerator.h"

int main (void) {

  //we re not in a power critical environment
  //but nevertheless switch off everything
  power_all_disable();
  //reset ports & pins
  DDRB = 0;
  PORTB = 0;
  DDRA = 0;
  PORTB = 0;

  //Start everything
  led_init();
  led_on();
  uint8_t result = accelerator_init();
  if (result==0) {
    led_off();
    led_start();
    //and let the games begin
    sei();
  }

  while (1) {
    led_calculate();
  }

}

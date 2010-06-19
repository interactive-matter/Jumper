/*
 * communication.c
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
 *  Created on: 19.05.2010
 */

#include <avr/interrupt.h>

#include "state.h"

#define NORTH_PIN _BV(0) //b
#define SOUTH_PIN _BV(1) //b
#define WEST_PIN _BV(2) //b
#define EAST_PIN _BV(2) //a

//interrupt masks to check if we are interested into the pin change
//PCMSK1
#define NORTH_INTERRUPT _BV(0)
#define SOUTH_INTERRUPT _BV(1)
#define WEST_INTERRUPT _BV(2)
//PCMSK0
#define EAST_INTERRUPT _BV(2)

void communication_init(void) {
  //per default the communication pins are inputs whith pull up enable
  DDRB &= ~(NORTH_PIN | SOUTH_PIN | WEST_PIN);
  DDRA &= ~EAST_PIN;
  PORTB |= NORTH_PIN | SOUTH_PIN | WEST_PIN;
  PORTB |= EAST_PIN;
  //enable the pin change interrupts
  PCMSK0 |= EAST_INTERRUPT;
  PCMSK1 |= NORTH_INTERRUPT | SOUTH_INTERRUPT | WEST_INTERRUPT;
}

void communication_propagate(uint8_t value) {

}

//TODO how do we recognize all inputs and send outputs in the same time
/* solution would be automatically time slicing, so that all nodes on a net synchronize
 * we have to prevent sending while we are receiving
 * we must send short enough to receive long enough
 * while getting every event
 * send bits instead of pwm - sigma delta modulation or so
 * while receiving bits we stop sending
 * pin changes are directly mapped to addition in led value
 * receiving disables sending, by just disabling the interrupt
 * before sending a collision detections prevents from sending the bit
 * sending will be done in main to be interruptible
 * sending disables receiving
 * so sending must be real fast
 * so we need no timing?
 */

ISR(PCINT0_vect)
{
}

//we map PCINT1 to PCINT0 - since we want one intterrupt routine
ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));

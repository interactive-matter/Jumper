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
#include <avr/power.h>

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

state_t state_sending;
state_t state_north_counting;
state_t state_south_counting;
state_t state_east_counting;
state_t state_west_counting;
state_t state_wants_to_send;

//our values
uint8_t north;
uint8_t south;
uint8_t west;
uint8_t east;

volatile uint8_t send_value = 0;

void
communication_init(void)
{
  //we first register our states
  state_sending = state_register_state();
  state_north_counting = state_register_state();
  state_south_counting = state_register_state();
  state_east_counting = state_register_state();
  state_west_counting = state_register_state();
  state_wants_to_send = state_register_state();

  //per default the communication pins are inputs with pull up enable
  DDRB &= ~(NORTH_PIN | SOUTH_PIN | WEST_PIN);
  DDRA &= ~EAST_PIN;
  PORTB |= NORTH_PIN | SOUTH_PIN | WEST_PIN;
  PORTB |= EAST_PIN;
  //enable the pin change interrupts
  PCMSK0 |= EAST_INTERRUPT;
  PCMSK1 |= NORTH_INTERRUPT | SOUTH_INTERRUPT | WEST_INTERRUPT;
  //timer 1 is our counter to decode numbers
  //we set it to mode 4 CTC (Clear Timer on Compare)
  //we count from 0 to 7F to spare the last bit sign, simulating 8 bit
  //and fastest mode
  TCCR1A = 0;
  TCCR1B = _BV(3) | _BV(0);
  OCR1A = 0x7F;
  //ok & we want an overflow interrupt
  TIFR1 = _BV(0);
  power_timer1_enable();
}
// the communication protocol goes like this:
// a sender can request sending by drawing the line to 0 shortly
// after a complete cycle it send 0 as less than half the duty cycle
// or 1 for more than half a duty cycle.
//at the end of the cycle it mast be drawn high for some cycles

//half a cycle is a bit shorter
#define HALF_CYCLE 60
#define ZERO_TIMEOUT 20
#define HIGH_TIMEOUT 100

int8_t north_begin;
uint8_t north_bit;
uint8_t north_value;

int8_t south_begin;
uint8_t south_bit;
uint8_t south_value;

int8_t west_begin;
uint8_t west_bit;
uint8_t west_value;

int8_t east_begin;
uint8_t east_bit;
uint8_t east_value;

uint8_t send_bit;

void
communication_propagate(uint8_t value)
{
  send_value = value;
  state_activate(state_wants_to_send);
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
  //if we are sending we ignore all state changes all
  if (!state_is_active(state_sending))
    {
      uint8_t portb = PINB;
      uint8_t porta = PINA;
      if (!state_is_active(state_north_counting))
        {
          if (!(portb & NORTH_PIN))
            {
              north_begin = TCNT1;
              north_bit = 0;
              north_value = 0;
            }
        }
      else
        {
          if (portb & NORTH_PIN)
            {
              int8_t length = TCNT1 - north_begin;
              //overflow?
              if (length < 0)
                {
                  length = 0x7f - length;
                }
              if (length > HALF_CYCLE)
                {
                  north_value |= _BV(north_bit);
                }
              north_bit++;
              //we simply assume that never more than 8 bits are send
              if (north_bit == 8) {
                state_deactivate(state_north_counting);
                north = north_value;
              }
            }
        }
      if (!state_is_active(state_south_counting))
        {
          if (!(portb & SOUTH_PIN))
            {
              south_begin = TCNT1;
              south_bit = 0;
              south_value = 0;
            }
        }
      else
        {
          if (portb & SOUTH_PIN)
            {
              int8_t length = TCNT1 - south_begin;
              //overflow?
              if (length < 0)
                {
                  length = 0x7f - length;
                }
              if (length > HALF_CYCLE)
                {
                  south_value |= _BV(south_bit);
                }
              south_bit++;
              //we simply assume that never more than 8 bits are send
              if (south_bit == 8) {
                state_deactivate(state_south_counting);
                south = south_value;
              }
            }
        }
      if (!state_is_active(state_west_counting))
        {
          if (!(portb & WEST_PIN))
            {
              west_begin = TCNT1;
              west_bit = 0;
              west_value = 0;
            }
        }
      else
        {
          if (portb & WEST_PIN)
            {
              int8_t length = TCNT1 - west_begin;
              //overflow?
              if (length < 0)
                {
                  length = 0x7f - length;
                }
              if (length > HALF_CYCLE)
                {
                  west_value |= _BV(west_bit);
                }
              west_bit++;
              //we simply assume that never more than 8 bits are send
            }
        }
      if (!state_is_active(state_east_counting))
        {
          if (!(porta & EAST_PIN))
            {
              east_begin = TCNT1;
              east_bit = 0;
              east_value = 0;
            }
        }
      else
        {
          if (porta & EAST_PIN)
            {
              int8_t length = TCNT1 - east_begin;
              //overflow?
              if (length < 0)
                {
                  length = 0x7f - length;
                }
              if (length > HALF_CYCLE)
                {
                  east_value |= _BV(east_bit);
                }
              east_bit++;
              //we simply assume that never more than 8 bits are send
            }
        }
    }
}

ISR(TIM1_OVF_vect)
{
  if (state_is_active(state_wants_to_send) &&
      !(state_is_active(state_north_counting) |
        state_is_active(state_south_counting) |
        state_is_active(state_east_counting) |
        state_is_active(state_west_counting)))
  if (state_is_active(state_sending)) {
    state_activate(state_sending);
    TIMSK1 |= OCIE1B;
    send_bit=0;
  }
  if (send_bit<8) {
    PORTB &= ~(NORTH_PIN | SOUTH_PIN | WEST_PIN);
    PORTA &= ~(EAST_PIN);
    if (send_value & _BV(send_bit)) {
      OCR1B=HIGH_TIMEOUT;
    } else {
      OCR1B=ZERO_TIMEOUT;
    }
    send_bit++;
  } else {
    state_deactivate(state_sending);
    TIMSK1 &= ~OCIE1B;
  }
}

ISR(TIM1_COMPB_vect ) {
  PORTB |= NORTH_PIN | SOUTH_PIN | WEST_PIN;
  PORTA |= EAST_PIN;
}

//we map PCINT1 to PCINT0 - since we want one interrupt routine
ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect))
;

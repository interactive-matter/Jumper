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

#include "i2cmaster.h"

#define ACCELERATOR_CS _BV(0)
#define ACCELERATOR_ADDRESS_PIN _BV(6)
#define ACCELERATOR_INTERRUPT_PIN _BV(4)
#define ACCELERATOR_VOLTAGE_REFERENCE 0 // _BV(6) would be internal
#define ACCELERATOR_I2C_ADDRESS 0x1d
#define ACCELERATOR_ID 0xE5
#define ACCELERATOR_RANGE 3 //0 = 2g, 1 = 4g, 3 = 8g, 4 = 16g

void
accelerator_write_register(uint8_t register_address, uint8_t value);

uint8_t
accelerator_read_register(uint8_t register_address);

int8_t accelerator_init(void) {
  //the configuration pins of the adxl are outputs
  DDRA |= ACCELERATOR_CS | ACCELERATOR_ADDRESS_PIN;
  DDRA &= ~(ACCELERATOR_INTERRUPT_PIN);
  //TODO enable pull up for i2c??
  //initialize i2C
  i2c_init();
  uint8_t id = accelerator_read_register(0x00);
  if (!id==ACCELERATOR_ID) {
    return -1;
  }
  //select the measurement range
  accelerator_write_register(0x31,ACCELERATOR_RANGE);
  //enable measurement
  accelerator_write_register(0x2d,_BV(3));
  return 0;
}

uint8_t
accelerator_read()
{
  //TODO is there a way to write directly to a struct?
  uint8_t hi;
  uint8_t lo;
  uint16_t result = 0;
  //we simply average all acceleration values
  //TODO come up with a better idea!
  i2c_start_wait(I2C_7BIT_WRITE(ACCELERATOR_I2C_ADDRESS));
  i2c_write(0x32);
  i2c_rep_start(I2C_7BIT_READ(ACCELERATOR_I2C_ADDRESS));
  hi = i2c_readAck();
  lo = i2c_readAck();
  result += (hi << 8 | lo);
  hi = i2c_readAck();
  lo = i2c_readAck();
  result += (hi << 8 | lo);
  i2c_stop();
  //average the result
  result /=3; //this send us back to 10 bit
  //and give back the upper 8 bit of the result
  return (result >> 2);
}

void
accelerator_write_register(uint8_t register_address, uint8_t value)
{
  i2c_start_wait(I2C_7BIT_WRITE(ACCELERATOR_I2C_ADDRESS));
  i2c_write(register_address);
  i2c_write(value);
  i2c_stop();
}

uint8_t
accelerator_read_register(uint8_t register_address)
{
  uint8_t result = -1;
  i2c_start_wait(I2C_7BIT_WRITE(ACCELERATOR_I2C_ADDRESS));
  i2c_write(register_address);
  i2c_rep_start(I2C_7BIT_READ(ACCELERATOR_I2C_ADDRESS));
  result = i2c_readNak();
  i2c_stop();
  return result;
}


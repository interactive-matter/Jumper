/*
 * status.h
 *
 *  Created on: 22.11.2009
 *      Author: marcus
 */

#ifndef STATUS_H_
#define STATUS_H_

extern volatile uint8_t state;

typedef void(*state_callback)(void);
typedef uint8_t state_t;

state_t state_register_task(state_callback callback);
state_t state_register_state();
void state_process(void);

#define state_activate(state_number) state |= state_number;
#define state_deactivate(state_number) state &= ~(state_number);
#define state_is_active(state_number) (state & state_number)


#endif /* STATUS_H_ */

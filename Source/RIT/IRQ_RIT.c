/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include <stdbool.h>
#include "../Main.h"

extern uint8_t game_state;
extern uint8_t soft_drop_active;
extern uint32_t tick;

static uint32_t last_move_left = 0;
static uint32_t last_move_right = 0;
static uint32_t last_rotate = 0;

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
void RIT_IRQHandler (void) {
    
    if (game_state != STATE_PLAYING) {
        soft_drop_active = 0;
        LPC_RIT->RICTRL |= 0x1;
        return;
    }
    
    // Joystick LEFT - faster debounce for simulator
    if ((LPC_GPIO1->FIOPIN & (1<<27)) == 0) {
        if ((tick - last_move_left) > 15) {  // 15 ticks = 150ms with 10ms tick
            move_left();
            last_move_left = tick;
        }
    }
    
    // Joystick RIGHT
    if ((LPC_GPIO1->FIOPIN & (1<<28)) == 0) {
        if ((tick - last_move_right) > 15) {
            move_right();
            last_move_right = tick;
        }
    }
    
    // Joystick UP - Rotate
    if ((LPC_GPIO1->FIOPIN & (1<<29)) == 0) {
        if ((tick - last_rotate) > 20) {  // 20 ticks = 200ms
            rotate_piece();
            last_rotate = tick;
        }
    }
    
    // Joystick DOWN - Soft drop (doubles current speed)
    if ((LPC_GPIO1->FIOPIN & (1<<26)) == 0) {
        soft_drop_active = 1;
    } else {
        soft_drop_active = 0;
    }
    
    LPC_RIT->RICTRL |= 0x1;  // Clear interrupt flag
}

/******************************************************************************
**                            End Of File
******************************************************************************/
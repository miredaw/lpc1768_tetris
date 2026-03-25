/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include <stdbool.h>
#include "../Main.h"

extern uint32_t tick;
bool toggle = false;

// Sound generation variables
static uint8_t sound_playing = 0;
static uint32_t sound_duration = 0;
static uint32_t sound_tick = 0;
static uint16_t sound_frequency = 0;

void TIMER0_IRQHandler (void)
{
    // This creates a square wave on the DAC
    
    if (sound_playing) {
        toggle = !toggle;
        if (toggle) {
            DAC_write(512);  // High value
        } else {
            DAC_write(0);    // Low value
        }
        
        sound_tick++;
        if (sound_tick >= sound_duration) {
            sound_playing = 0;
            sound_tick = 0;
            DAC_write(0);
        }
    }

    LPC_TIM0->IR |= 1;  // Clear interrupt flag
    return;
}


void TIMER1_IRQHandler (void)
{
    LPC_TIM1->IR = 1;  // Clear interrupt flag
    return;
}

void TIMER2_IRQHandler (void)
{
    LPC_TIM2->IR = 1;  // Clear interrupt flag
    return;
}

void TIMER3_IRQHandler (void)
{
    tick++;
    LPC_TIM3->IR = 1;  // Clear interrupt flag
    return;
}

// Sound helper functions (for hardware only)
void start_sound(uint16_t frequency, uint32_t duration_ms) {
    sound_frequency = frequency;
    sound_duration = duration_ms * 10;  // Convert to timer ticks
    sound_tick = 0;
    sound_playing = 1;
}

void play_sound_line_clear(void) {
    start_sound(1000, 100);  // 1kHz for 100ms
}

void play_sound_tetris(void) {
    start_sound(1500, 200);  // 1.5kHz for 200ms
}

void play_sound_game_over(void) {
    start_sound(500, 500);   // 500Hz for 500ms
}

void play_sound_powerup(void) {
    start_sound(2000, 150);  // 2kHz for 150ms
}

/******************************************************************************
**                            End Of File
******************************************************************************/
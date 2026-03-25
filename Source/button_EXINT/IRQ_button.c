#include "button.h"
#include "LPC17xx.h"
#include "GLCD/GLCD.h"

#include "../led/led.h"
#include "../timer/timer.h"
#include "RIT.h"
#include "../Main.h"
extern uint8_t game_state;
extern uint32_t tick;
extern uint32_t last_fall_tick;

void EINT0_IRQHandler (void) {
    // KEY1 - Start/Pause/Restart
    
    if (game_state == STATE_PAUSED || game_state == STATE_GAMEOVER) {
        init_game();
        game_state = STATE_PLAYING;
        last_fall_tick = tick;
    } else if (game_state == STATE_PLAYING) {
        game_state = STATE_PAUSED;
        GUI_Text(60, 150, (uint8_t*)"PAUSED", Yellow, Black);
    }
    
    LPC_SC->EXTINT &= (1 << 0);
}

void EINT1_IRQHandler (void) {
    // KEY2 - Hard drop
    
    if (game_state == STATE_PLAYING) {
        hard_drop();
    }
    
    LPC_SC->EXTINT &= (1 << 1);
}

void EINT2_IRQHandler (void) {
    // Not used
    
    LPC_SC->EXTINT &= (1 << 2);
}
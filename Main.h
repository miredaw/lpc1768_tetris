#include "LPC17xx.h"
#include "timer/timer.h"
#include "button_EXINT/button.h"
#include "led/led.h"
#include "joystick/joystick.h"
#include "RIT/RIT.h"
#include "ADC/adc.h"
#include "GLCD/GLCD.h"
#include "TouchPanel/TouchPanel.h"
#include "DAC/DAC.h"
#include <stdio.h>
#include <stdlib.h>

// Game constants
#define FIELD_WIDTH 14
#define FIELD_HEIGHT 25
#define BLOCK_SIZE 10
#define FIELD_X 10
#define FIELD_Y 10

// Game states
#define STATE_PAUSED 0
#define STATE_PLAYING 1
#define STATE_GAMEOVER 2

// Tetromino colors
#define COLOR_I Cyan
#define COLOR_O Yellow
#define COLOR_T Magenta
#define COLOR_S Green
#define COLOR_Z Red
#define COLOR_J Blue
#define COLOR_L 0xFD20  // Orange

// Powerup types
#define POWERUP_NONE 0
#define POWERUP_CLEAR_HALF 1
#define POWERUP_SLOW_DOWN 2
#define POWERUP_COLOR 0xFFFF  // White for powerup blocks

// Game structure
typedef struct {
    uint8_t shape;
    uint8_t rotation;
    int16_t x;
    int16_t y;
    uint16_t color;
} Tetromino;

// Global variables
extern uint8_t game_field[FIELD_HEIGHT][FIELD_WIDTH];
extern uint8_t powerup_field[FIELD_HEIGHT][FIELD_WIDTH];
extern Tetromino current_piece;
extern uint8_t game_state;
extern uint32_t score;
extern uint32_t high_score;
extern uint32_t lines_cleared;
extern uint32_t lines_since_powerup;
extern uint32_t lines_since_malus;
extern uint8_t fall_speed;
extern uint32_t last_fall_tick;
extern uint8_t soft_drop_active;
extern uint16_t adc_speed_value;
extern uint8_t slow_down_active;
extern uint32_t slow_down_start_tick;
extern uint16_t AD_current;

// Function prototypes
void init_game(void);
void spawn_tetromino(void);
void draw_field(void);
void draw_tetromino(Tetromino *piece, uint16_t color);
uint8_t check_collision(Tetromino *piece);
void place_tetromino(void);
void clear_lines(void);
void move_left(void);
void move_right(void);
void rotate_piece(void);
void move_down(void);
void hard_drop(void);
void update_score_display(void);
void game_over(void);
uint16_t get_tetromino_color(uint8_t shape);

//Powerup functions
void spawn_powerup(void);
void activate_powerup(uint8_t type);
void clear_half_lines(void);
void activate_slow_down(void);
void update_slow_down_timer(void);

// Malus function
void add_malus_line(void);

//Sound functions (for hardware only)
void play_sound_line_clear(void);
void play_sound_tetris(void);
void play_sound_game_over(void);
void play_sound_powerup(void);
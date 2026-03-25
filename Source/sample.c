#include "../Main.h"

extern uint32_t tick;

// Global game variables
uint8_t game_field[FIELD_HEIGHT][FIELD_WIDTH] = {0};
uint8_t powerup_field[FIELD_HEIGHT][FIELD_WIDTH] = {0};
Tetromino current_piece;
uint8_t game_state = STATE_PAUSED;
uint32_t score = 0;
uint32_t high_score = 0;
uint32_t lines_cleared = 0;
uint32_t lines_since_powerup = 0;
uint32_t lines_since_malus = 0;
uint8_t fall_speed = 1;
uint32_t last_fall_tick = 0;
uint8_t soft_drop_active = 0;
uint16_t adc_speed_value = 1;
uint8_t slow_down_active = 0;
uint32_t slow_down_start_tick = 0;

uint16_t get_tetromino_color(uint8_t shape) {
    switch(shape) {
        case TETRO_I: return COLOR_I;
        case TETRO_O: return COLOR_O;
        case TETRO_T: return COLOR_T;
        case TETRO_S: return COLOR_S;
        case TETRO_Z: return COLOR_Z;
        case TETRO_J: return COLOR_J;
        case TETRO_L: return COLOR_L;
        default: return White;
    }
}

void init_game(void) {
    uint8_t i, j;
    
    // Clear field
    for (i = 0; i < FIELD_HEIGHT; i++) {
        for (j = 0; j < FIELD_WIDTH; j++) {
            game_field[i][j] = 0;
            powerup_field[i][j] = POWERUP_NONE;
        }
    }
    
    score = 0;
    lines_cleared = 0;
    lines_since_powerup = 0;
    lines_since_malus = 0;
    fall_speed = 1;
    soft_drop_active = 0;
    slow_down_active = 0;
    last_fall_tick = tick;
    
    LCD_Clear(Black);
    draw_field();
    update_score_display();
    
    spawn_tetromino();
}

void spawn_tetromino(void) {
    current_piece.shape = rand() % 7;
    current_piece.rotation = 0;
    current_piece.x = FIELD_WIDTH / 2 - 2;
    current_piece.y = 0;
    current_piece.color = get_tetromino_color(current_piece.shape);
    
    if (check_collision(&current_piece)) {
        game_over();
    } else {
        draw_tetromino(&current_piece, current_piece.color);
    }
}

void draw_field(void) {
    uint8_t i, j, x, y;
    
    // Draw border
    for (i = 0; i < FIELD_HEIGHT * BLOCK_SIZE + 2; i++) {
        LCD_SetPoint(FIELD_X - 1, FIELD_Y + i, White);
        LCD_SetPoint(FIELD_X + FIELD_WIDTH * BLOCK_SIZE, FIELD_Y + i, White);
    }
    for (i = 0; i < FIELD_WIDTH * BLOCK_SIZE + 2; i++) {
        LCD_SetPoint(FIELD_X + i - 1, FIELD_Y - 1, White);
        LCD_SetPoint(FIELD_X + i - 1, FIELD_Y + FIELD_HEIGHT * BLOCK_SIZE, White);
    }
    
    // Draw placed blocks
    for (i = 0; i < FIELD_HEIGHT; i++) {
        for (j = 0; j < FIELD_WIDTH; j++) {
            uint16_t color = Black;
            
            // Check if this is a powerup block
            if (powerup_field[i][j] != POWERUP_NONE) {
                color = POWERUP_COLOR;  // White for powerup
            } else if (game_field[i][j] != 0) {
                color = get_tetromino_color(game_field[i][j] - 1);
            }
            
            for (x = 0; x < BLOCK_SIZE; x++) {
                for (y = 0; y < BLOCK_SIZE; y++) {
                    LCD_SetPoint(FIELD_X + j * BLOCK_SIZE + x, 
                               FIELD_Y + i * BLOCK_SIZE + y, 
                               color);
                }
            }
        }
    }
}

void draw_tetromino(Tetromino *piece, uint16_t color) {
    PutTetrominoes(FIELD_X + piece->x * BLOCK_SIZE, 
                   FIELD_Y + piece->y * BLOCK_SIZE, 
                   piece->shape, 
                   piece->rotation, 
                   BLOCK_SIZE, 
                   color, 
                   Black);
}

uint8_t check_collision(Tetromino *piece) {
    uint8_t i;
    
    for (i = 0; i < 4; i++) {
        int8_t block_x = tetromino_shapes[piece->shape][piece->rotation][i][0];
        int8_t block_y = tetromino_shapes[piece->shape][piece->rotation][i][1];
        
        int16_t test_x = piece->x + block_x;
        int16_t test_y = piece->y + block_y;
        
        // Check boundaries
        if (test_x < 0 || test_x >= FIELD_WIDTH || test_y >= FIELD_HEIGHT) {
            return 1;
        }
        
        // Check collision with placed blocks (only if y >= 0)
        if (test_y >= 0 && game_field[test_y][test_x] != 0) {
            return 1;
        }
    }
    
    return 0;
}

void place_tetromino(void) {
    uint8_t i;
    
    // Place current piece in field array
    for (i = 0; i < 4; i++) {
        int8_t block_x = tetromino_shapes[current_piece.shape][current_piece.rotation][i][0];
        int8_t block_y = tetromino_shapes[current_piece.shape][current_piece.rotation][i][1];
        
        int16_t field_x = current_piece.x + block_x;
        int16_t field_y = current_piece.y + block_y;
        
        if (field_y >= 0 && field_y < FIELD_HEIGHT && field_x >= 0 && field_x < FIELD_WIDTH) {
            game_field[field_y][field_x] = current_piece.shape + 1;
        }
    }
    
    score += 10;
    clear_lines();
    spawn_tetromino();
    update_score_display();
}

//Spawn powerup on random existing block
void spawn_powerup(void) {
    uint8_t attempts = 0;
    uint8_t found = 0;
    uint8_t i, j;
    
    // Try to find a random block to replace with powerup
    while (attempts < 100 && !found) {
        i = rand() % FIELD_HEIGHT;
        j = rand() % FIELD_WIDTH;
        
        if (game_field[i][j] != 0 && powerup_field[i][j] == POWERUP_NONE) {
            // Random powerup type: 50% clear half, 50% slow down
            powerup_field[i][j] = (rand() % 2) + 1;
            found = 1;
        }
        attempts++;
    }
}

//Clear bottom half of lines
void clear_half_lines(void) {
    uint8_t i, j;
    uint8_t total_lines = 0;
    uint8_t lines_to_clear;
    
    // Count total lines with blocks
    for (i = 0; i < FIELD_HEIGHT; i++) {
        uint8_t has_block = 0;
        for (j = 0; j < FIELD_WIDTH; j++) {
            if (game_field[i][j] != 0) {
                has_block = 1;
                break;
            }
        }
        if (has_block) total_lines++;
    }
    
    lines_to_clear = total_lines / 2;
    
    // Clear from bottom
    uint8_t cleared = 0;
    for (i = FIELD_HEIGHT - 1; i >= 0 && cleared < lines_to_clear; i--) {
        uint8_t has_block = 0;
        for (j = 0; j < FIELD_WIDTH; j++) {
            if (game_field[i][j] != 0) {
                has_block = 1;
                break;
            }
        }
        
        if (has_block) {
            // Clear this line
            for (j = 0; j < FIELD_WIDTH; j++) {
                game_field[i][j] = 0;
                powerup_field[i][j] = POWERUP_NONE;
            }
            cleared++;
            
            // Award points in groups of 4
            if (cleared % 4 == 0) {
                score += 600;  // Tetris bonus
            } else if (cleared == lines_to_clear) {
                // Last group (might be less than 4)
                uint8_t remainder = cleared % 4;
                if (remainder > 0) {
                    score += remainder * 100;
                }
            }
        }
    }
}

//Activate slow down powerup
void activate_slow_down(void) {
    slow_down_active = 1;
    slow_down_start_tick = tick;
}

//Update slow down timer
void update_slow_down_timer(void) {
    if (slow_down_active) {
        // Check if 15 seconds (1500 ticks) have passed
        if ((tick - slow_down_start_tick) >= 1500) {
            slow_down_active = 0;
        }
    }
}

//Activate powerup effect
void activate_powerup(uint8_t type) {
    if (type == POWERUP_CLEAR_HALF) {
        clear_half_lines();
        play_sound_powerup();
    } else if (type == POWERUP_SLOW_DOWN) {
        if (adc_speed_value > 1) {  // Only activate if speed > 1
            activate_slow_down();
            play_sound_powerup();
        }
    }
}

void clear_lines(void) {
    uint8_t i, j, k;
    uint8_t lines_found = 0;
    uint8_t powerups_activated[FIELD_HEIGHT] = {0};
    
    // Find full lines and check for powerups
    for (i = 0; i < FIELD_HEIGHT; i++) {
        uint8_t full = 1;
        for (j = 0; j < FIELD_WIDTH; j++) {
            if (game_field[i][j] == 0) {
                full = 0;
                break;
            }
        }
        
        if (full) {
            // Check if line contains powerup
            for (j = 0; j < FIELD_WIDTH; j++) {
                if (powerup_field[i][j] != POWERUP_NONE) {
                    powerups_activated[i] = powerup_field[i][j];
                }
            }
            
            lines_found++;
            // Shift lines down
            for (k = i; k > 0; k--) {
                for (j = 0; j < FIELD_WIDTH; j++) {
                    game_field[k][j] = game_field[k-1][j];
                    powerup_field[k][j] = powerup_field[k-1][j];
                }
            }
            // Clear top line
            for (j = 0; j < FIELD_WIDTH; j++) {
                game_field[0][j] = 0;
                powerup_field[0][j] = POWERUP_NONE;
            }
        }
    }
    
    if (lines_found > 0) {
        lines_cleared += lines_found;
        lines_since_powerup += lines_found;
        lines_since_malus += lines_found;
        
        // Score calculation according to spec
        if (lines_found == 4) {
            score += 600;  // Tetris bonus
            play_sound_tetris();
        } else {
            score += lines_found * 100;
            play_sound_line_clear();
        }
        
        //Check for powerup spawn (every 5 lines)
        if (lines_since_powerup >= 5) {
            spawn_powerup();
            lines_since_powerup = 0;
        }
        
        //Check for malus (every 10 lines)
        if (lines_since_malus >= 10) {
            add_malus_line();
            lines_since_malus = 0;
        }
        
        //Activate powerups that were in cleared lines
        for (i = 0; i < FIELD_HEIGHT; i++) {
            if (powerups_activated[i] != POWERUP_NONE) {
                activate_powerup(powerups_activated[i]);
            }
        }
        
        draw_field();
    }
}

//Add malus line at bottom
void add_malus_line(void) {
    uint8_t i, j;
    uint8_t blocks[FIELD_WIDTH];
    uint8_t placed = 0;
    
    // Shift all lines up by 1
    for (i = 0; i < FIELD_HEIGHT - 1; i++) {
        for (j = 0; j < FIELD_WIDTH; j++) {
            game_field[i][j] = game_field[i+1][j];
            powerup_field[i][j] = powerup_field[i+1][j];
        }
    }
    
    // Create new bottom line with 7 random blocks
    for (j = 0; j < FIELD_WIDTH; j++) {
        blocks[j] = 0;
    }
    
    while (placed < 7) {
        j = rand() % FIELD_WIDTH;
        if (blocks[j] == 0) {
            blocks[j] = (rand() % 7) + 1;  // Random tetromino type
            placed++;
        }
    }
    
    // Place the new line at bottom
    for (j = 0; j < FIELD_WIDTH; j++) {
        game_field[FIELD_HEIGHT - 1][j] = blocks[j];
        powerup_field[FIELD_HEIGHT - 1][j] = POWERUP_NONE;
    }
    
    // Check if top line has blocks (game over)
    for (j = 0; j < FIELD_WIDTH; j++) {
        if (game_field[0][j] != 0) {
            game_over();
            return;
        }
    }
}

void move_left(void) {
    if (game_state != STATE_PLAYING) return;
    
    draw_tetromino(&current_piece, Black);
    current_piece.x--;
    if (check_collision(&current_piece)) {
        current_piece.x++;
    }
    draw_tetromino(&current_piece, current_piece.color);
}

void move_right(void) {
    if (game_state != STATE_PLAYING) return;
    
    draw_tetromino(&current_piece, Black);
    current_piece.x++;
    if (check_collision(&current_piece)) {
        current_piece.x--;
    }
    draw_tetromino(&current_piece, current_piece.color);
}

void rotate_piece(void) {
    if (game_state != STATE_PLAYING) return;
    
    draw_tetromino(&current_piece, Black);
    uint8_t old_rotation = current_piece.rotation;
    current_piece.rotation = (current_piece.rotation + 1) % 4;
    if (check_collision(&current_piece)) {
        current_piece.rotation = old_rotation;
    }
    draw_tetromino(&current_piece, current_piece.color);
}

void move_down(void) {
    if (game_state != STATE_PLAYING) return;
    
    draw_tetromino(&current_piece, Black);
    current_piece.y++;
    if (check_collision(&current_piece)) {
        current_piece.y--;
        draw_tetromino(&current_piece, current_piece.color);
        place_tetromino();
    } else {
        draw_tetromino(&current_piece, current_piece.color);
    }
}

void hard_drop(void) {
    if (game_state != STATE_PLAYING) return;
    
    draw_tetromino(&current_piece, Black);
    while (1) {
        current_piece.y++;
        if (check_collision(&current_piece)) {
            current_piece.y--;
            break;
        }
    }
    draw_tetromino(&current_piece, current_piece.color);
    place_tetromino();
}

void update_score_display(void) {
    char buffer[20];
    
    sprintf(buffer, "TOP");
    GUI_Text(175, 240, (uint8_t*)buffer, White, Black);
    sprintf(buffer, "%06u", (unsigned int)high_score);
    GUI_Text(175, 256, (uint8_t*)buffer, White, Black);
    
    sprintf(buffer, "SCORE");
    GUI_Text(175, 280, (uint8_t*)buffer, White, Black);
    sprintf(buffer, "%06u", (unsigned int)score);
    GUI_Text(175, 296, (uint8_t*)buffer, White, Black);
}

void game_over(void) {
    game_state = STATE_GAMEOVER;
    
    if (score > high_score) {
        high_score = score;
        update_score_display();
    }
    
    play_sound_game_over();
    
    GUI_Text(110, 150, (uint8_t*)"GAME IS OVER", Red, Black);
    GUI_Text(100, 170, (uint8_t*)"Press INT0", White, Black);
}

int main() {
    SystemInit();

    // Initialize Timer 3 for game tick (10ms)
    init_timer_SRI(3, 250000, 0b011);  // 10ms tick
    enable_timer(3);
    
    //Initialize Timer 0 for sound
    init_timer_SRI(0, 25000, 0b011);  // Sound timer
    enable_timer(0);
    
    // Initialize LCD
    LCD_Initialization();
    LCD_Clear(Black);
    
    // Initialize peripherals
    BUTTON_init();
    joystick_init();
    
    //Initialize ADC for speed control
    ADC_init();
    ADC_start_conversion();
    
    //Initialize DAC for sound
    DAC_init();
    
    // Initialize RIT for joystick polling
    init_RIT(0x00989680);  // ~10ms for joystick
    enable_RIT();
    
    // Seed random number generator
    srand(12345);
    
    // Initial screen
    GUI_Text(60, 140, (uint8_t*)"TETRIS Game By Milad", White, Black);
    GUI_Text(50, 160, (uint8_t*)"Press INT0", White, Black);
    
    update_score_display();
    
    while(1) {
        if (game_state == STATE_PLAYING) {
            // Update slow down timer
            update_slow_down_timer();
            
            // Calculate fall interval based on ADC speed and slow down
            uint32_t current_speed = adc_speed_value;
            
            // If slow down is active, force speed to 1
            if (slow_down_active) {
                current_speed = 1;
            }
            
            // Soft drop doubles current speed
            if (soft_drop_active) {
                current_speed *= 2;
            }
            
            // Convert speed to fall interval (ticks between falls)
            // Speed 1 = 100 ticks (1000ms)
            // Speed 2 = 50 ticks (500ms)
            // Speed 3 = 33 ticks (333ms)
            // Speed 4 = 25 ticks (250ms)
            // Speed 5 = 20 ticks (200ms)
            uint32_t fall_interval = 100 / current_speed;
            
            if ((tick - last_fall_tick) >= fall_interval) {
                move_down();
                last_fall_tick = tick;
            }
            
            // Periodically start ADC conversion to update speed
            if ((tick % 10) == 0) {  // Every 100ms
                ADC_start_conversion();
            }
        }
    }
}
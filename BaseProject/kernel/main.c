#include "console.h"
#include "page.h"
#include "process.h"
#include "keyboard.h"
#include "mouse.h"
#include "interrupt.h"
#include "clock.h"
#include "ata.h"
#include "device.h"
#include "cdromfs.h"
#include "string.h"
#include "graphics.h"
#include "kernel/ascii.h"
#include "kernel/syscall.h"
#include "rtc.h"
#include "kernelcore.h"
#include "kmalloc.h"
#include "memorylayout.h"
#include "kshell.h"
#include "cdromfs.h"
#include "diskfs.h"
#include "serial.h"

#define SCREEN_WIDTH 1020
#define SCREEN_HEIGHT 700
#define PLAYER_WIDTH 60
#define PLAYER_HEIGHT 40

#define ENEMY_WIDTH 20
#define ENEMY_HEIGHT 12

#define MAX_BULLETS 10000 
#define BULLET_SPEED 20 
#define BULLET_WIDTH 7 
#define BULLET_HEIGHT 20 


int activeKey = 0;
int score = 0; // Skor değişkeni
int player_size = 60; // maximum player size = 80
int enemy_size = 60;
int level = 1;
int enemy_speed = 10;

typedef enum {
    LEFT,
    RIGHT,
    NONE
} Directions;

typedef struct {
    int x;
    int y;
    Directions directions;
} Player;

Player player;

typedef struct {
    int x;
    int y;
    Directions directions;
} Enemy;

Enemy enemy;

typedef struct {
    int x;
    int y;
    Directions directions;
} Bullet;

Bullet bullets[MAX_BULLETS]; // Mermilerin saklandığı dizi
int numBullets = 0; // Mevcut mermi sayısı


int simple_rand();
void clear_screen(struct graphics *g);
void printString(struct graphics *g, int x, int y, char *str);
void printInt(struct graphics *g, int x, int y, int value);
void keyboard_handler_main();
void move_player();
void draw_player(struct graphics *g);
void move_enemy(struct graphics *g);
void update_bullet();
void move_bullet();
void draw_bullet(struct graphics *g);
void start_screen(struct graphics *g);
void end_screen(struct graphics *g);
void check_collision(struct graphics *g);
void print_score_Integer(struct graphics *g);
void print_score(struct graphics *g);
void draw_triangle(struct graphics *g,int x, int y, int size);
void draw_hexagon(struct graphics *g,int x, int y, int size);
bool isCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2);
void print_level(struct graphics *g);
void print_level_Integer(struct graphics *g);


int simple_rand() 
{
    static unsigned int seed = 0xDEADBEEF;
    seed = (seed * 1103515245 + 12345) & 0x7fffffff;
    return seed;
}

void sleep(int milliseconds) 
{
    int i, j;
    for (i = 0; i < milliseconds; i++) {
        for (j = 0; j < 1000000; j++) {
            // Do nothing (waste CPU cycles)
        }
    }
}

void clear_screen(struct graphics *g) 
{
    // Clears the screen
    graphics_clear(g, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
}

void printString(struct graphics *g, int x, int y, char *str) 
{
    int i = 0;
    while (str[i] != '\0') {
        graphics_char(g, x + i * 8, y, str[i]);
        i++;
    }
}

void printInt(struct graphics *g, int x, int y, int value) 
{
    char buffer[12]; 
    int i = 0;
    if (value == 0) {
        graphics_char(g, x, y, '0');
        return;
    }

    if (value < 0) {
        graphics_char(g, x, y, '-');
        value *= -1;
        x += 8; 
    }

    while (value != 0) {
        buffer[i++] = '0' + (value % 10);
        value /= 10;
    }

    while (i > 0) {
        graphics_char(g, x, y, buffer[--i]);
        x += 8;
    }
}

void draw_triangle(struct graphics *g,int x, int y, int size)
{	
	
	for(int i = 0;i <= size; i++)
	{
	graphics_line(g, x-(size/2) + i, y +(size/ 2) - i, size * 2 - (2*i), 0); // Üçgenin tabanını oluşturan çizgi ( x, y, w, h)
	}
}

void draw_hexagon(struct graphics *g,int x, int y, int size)
{    
    int hexagon_top = size;
    int hexagon_middle = size*2;
    
    for(int i = 0; i < hexagon_top/2; i++)
    {
    	graphics_line(g, x - i, y + i, hexagon_top + i * 2, 0); // Hexagonun ilk yarisi
    }
    
    x = x - hexagon_top/2;
    y = y + hexagon_top/2;
    
    for(int j = 0; j < hexagon_top/2; j++)
    {
    	graphics_line(g, x + j, y + j, hexagon_middle - (j*2), 0); // Hexagonun ikinci yarisi
    }
}


void keyboard_handler_main()
{
   char *key[2],left[2],right[2], space[2];
	key[0]=0;
	key[1] =0;
	
	left[0] = 97;
	left[1] = 0;
	right[0] =100;
	right[1] =0;
	space[0] = 32; // Space
    	space[1] = 0;
	
    
       key[0] = keyboard_read(1);
		key[1] = 0;
		if(key[0] == right[0] ){
			player.directions = RIGHT;
			activeKey = 1;

		}
		else if(key[0] == left[0] ){
			player.directions = LEFT;
			activeKey = 1;
		}
		else if(key[0] == space[0] ){
			update_bullet();
		}
}
void move_player() 
{
    if (player.directions == LEFT) {
        if (player.x > player_size) {
            player.x -= 20;
        }
    } else if (player.directions == RIGHT) {
        if (player.x < SCREEN_WIDTH - (player_size*2)) {
            player.x += 20;
        }
    }
}

void draw_player(struct graphics *g) 
{
    struct graphics_color color = {255, 255, 255}; // White
    graphics_fgcolor(g, color); // Set foreground color to White
    draw_triangle(g, player.x, player.y, player_size);
}

void move_enemy(struct graphics *g) 
{
    enemy.y += enemy_speed;

    if (enemy.y + enemy_size >= SCREEN_HEIGHT) { // Check if the enemy reached the bottom of the screen
	end_screen(g);
        while (1) {
            // Wait until game ends
        }
    }
}



void draw_enemy(struct graphics *g) 
{
    struct graphics_color color = {255, 0, 0, 0}; // red
    graphics_fgcolor(g, color); // Set foreground color to red

    // Draw the enemy as a filled rectangle
    draw_hexagon(g,enemy.x, enemy.y, enemy_size);
}

void update_bullet() 
{
    if (numBullets < MAX_BULLETS) {

        bullets[numBullets].x = player.x + player_size / 2 ; // Uzay gemisinin ortasına yerleştirin
        bullets[numBullets].y = player.y - player_size; // Uzay gemisinin üstüne yerleştirin
        bullets[numBullets].directions = NONE;
        numBullets++;
    }
}

void move_bullet() 
{
    for (int i = 0; i < numBullets; i++) {
        bullets[i].y -= BULLET_SPEED;
        if (bullets[i].y < 0) {
            bullets[i] = bullets[numBullets - 1];
            numBullets--; 
            i--;
        }
    }
}

void draw_bullet(struct graphics *g) 
{
    for (int i = 0; i < numBullets; i++) {
        struct graphics_color color = {0, 255, 0}; // Green
        graphics_fgcolor(g, color); // Set foreground color to Green
        graphics_rect(g, bullets[i].x, bullets[i].y, BULLET_WIDTH, BULLET_HEIGHT);
    }
}

void print_score(struct graphics *g)
{
        struct graphics_color color = {255, 255, 255}; // white
        graphics_fgcolor(g, color); // Set foreground color to white
	printString(g, 10, 10, "Score: "); // Score on the screen
}
void print_score_Integer(struct graphics *g)
{
	struct graphics_color color = {255, 255, 255}; // white
        graphics_fgcolor(g, color); // Set foreground color to white
	printInt(g, 70, 10, score);
}
void print_level(struct graphics *g)
{
        struct graphics_color color = {255, 255, 255}; // white
        graphics_fgcolor(g, color); // Set foreground color to white
	printString(g, 10, 30, "Level: "); // Score on the screen
}
void print_level_Integer(struct graphics *g)
{
	struct graphics_color color = {255, 255, 255}; // white
        graphics_fgcolor(g, color); // Set foreground color to white
	printInt(g, 70, 30, level);
}



void start_screen(struct graphics *g)
{
        clear_screen(g);
        
        struct graphics_color color = {255, 255, 255}; // white
        graphics_fgcolor(g, color); // Set foreground color to white
        
        printString(g, 50, 350, "Press Space to shoot.");
        printString(g, 50, 370, "Press A to move left, D to move right.");
        printString(g, 50, 390, "Press any key to start.");


}
void end_screen(struct graphics *g)
{
        clear_screen(g);
        
        struct graphics_color color = {255, 0, 0}; // white
        graphics_fgcolor(g, color); // Set foreground color to white
        
        printString(g, 510, 350, "GAME OVER!");
        
        struct graphics_color color2 = {255, 255, 255}; // white
        graphics_fgcolor(g, color2); // Set foreground color to white
        printString(g, 510, 370, "SCORE: ");
        printInt(g, 570, 370, score);

}
bool isCollision(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

void check_collision(struct graphics *g) 
{
     if (isCollision(player.x - player_size/2, player.y - player_size/2, player_size, player_size, enemy.x - enemy_size, enemy.y - enemy_size, 2 * enemy_size, 2 * enemy_size)) { // Checks if enemy and player has collision
        end_screen(g);
        while (1) {
            // Wait until game ends
        }
    }
    
    for (int i = 0; i < numBullets; i++) {

        if (bullets[i].x >= enemy.x - 40 && bullets[i].x <= enemy.x + enemy_size + 40 &&
           bullets[i].y > enemy.y && bullets[i].y < enemy.y + enemy_size ) { // Checks if enemy and bullet has collision
            score += 1;
            
            enemy.x = simple_rand() % (SCREEN_WIDTH - (enemy_size + 5)); // start enemy again
	    enemy.y = 0; 
	    enemy.directions = NONE;
            
            bullets[i] = bullets[numBullets - 1];
            numBullets--;
            i--;
        }
    }   
}

int kernel_main() {
    struct graphics *g = graphics_create_root();

    console_init(g);
    console_addref(&console_root);

    page_init();
    kmalloc_init((char *)KMALLOC_START, KMALLOC_LENGTH);
    interrupt_init();
    rtc_init();
    keyboard_init();
    process_init();
    
    // Players first position
    player.x = (SCREEN_WIDTH - player_size) / 2;
    player.y = SCREEN_HEIGHT - player_size - 1; // Placing it at the bottom of the console
    player.directions = NONE;
    
    // Enemys first position
    enemy.x = simple_rand() % (SCREEN_WIDTH - ENEMY_WIDTH);
    enemy.y = 0; 
    enemy.directions = NONE;

    start_screen(g);
    
    // Wait for any key press to start the game
    keyboard_read(0);

    while (1) {
        clear_screen(g);
        keyboard_handler_main();
        
        if (activeKey == 1) {
            move_player();
            activeKey = 0;
        }
        
        move_enemy(g);
        draw_player(g);
        draw_enemy(g);
        draw_bullet(g);
        move_bullet();
        
        check_collision(g); // Check all collisions
        
        print_score(g);
        print_score_Integer(g);
        print_level(g);
        print_level_Integer(g);
        
        if (score >= 50){
            enemy_speed = 10;
            level = 5;
        }
        else if (score >= 30){
            enemy_speed = 9;
            level = 4;
        }
        else if (score >= 20){
            enemy_speed = 8;
            level = 3;
        }
        else if (score >= 10){
            enemy_speed = 7;
            level = 2;
        }
        else{
            enemy_speed = 6;
            level = 1;
        }

        sleep(15); // sleep
    }
    
    return 0;
}


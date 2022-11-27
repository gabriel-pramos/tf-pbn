#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <util/delay.h>

#include "adc.h"
#include "nokia5110.h"
#include "print.h"
#include "usart.h"

#define WIDTH 70
#define HEIGHT 47

int aim_x = WIDTH / 2;
int aim_y = HEIGHT / 2;

int target_x = WIDTH / 2;
int target_y = HEIGHT / 2;
int target_radius = 10;

int points = 0;
int t = 30;

void draw_start();
void draw();
void draw_points();
void draw_target();
void draw_aim();

void draw_start()
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(0, 0);
    nokia_lcd_write_string("Press 's' to start", 1);
    nokia_lcd_render();
}

void draw()
{
    nokia_lcd_clear();
    nokia_lcd_drawrect(0, 0, WIDTH, HEIGHT);
    draw_points();
    draw_aim();
    draw_target();
    nokia_lcd_render();
}

void draw_points()
{
    nokia_lcd_set_cursor(73, 3);
    char sp[10];
    sprintf(sp, "%d", points);
    nokia_lcd_write_string(sp, 1);
    nokia_lcd_set_cursor(73, 30);
    char st[10];
    sprintf(st, "%d", t);
    nokia_lcd_write_string(st, 1);
}

void draw_aim()
{
    nokia_lcd_drawcircle(aim_x, aim_y, 5);
    nokia_lcd_drawline(aim_x - 5, aim_y, aim_x + 5, aim_y);
    nokia_lcd_drawline(aim_x, aim_y - 5, aim_x, aim_y + 5);
}

void draw_target()
{
    nokia_lcd_drawcircle(target_x, target_y, target_radius);
}

int main()
{
    // set pc2 as input
    DDRC &= ~(1 << DDC2);

    USART_Init();
    adc_init();

    nokia_lcd_init();

    draw_start();
    int r = 0;
    while (!(PINC & (1 << PC2)))
        r++;
    srand(r);

    for (;;) {
        draw();
        adc_set_channel(0);
        float x = adc_read();
        adc_set_channel(1);
        float y = adc_read();

        if (aim_x + ((x - 511) / 100) > 5 && aim_x + ((x - 511) / 100) < WIDTH - 5)
            aim_x += (x - 511) / 100;
        if (aim_y - ((y - 511) / 100) > 5 && aim_y - ((y - 511) / 100) < HEIGHT - 5)
            aim_y -= (y - 511) / 100;

        // print("\naim_x: ");
        // printfloat(aim_x);
        // print("  aim_y: ");
        // printfloat(aim_y);
        // print("\nsecond_count: ");
        // printfloat(second_count);

        // Código do simulador tem um BUG, sempre vai retornar
        // ON no estado do switch
        // if (PINC & (1 << PC2))
        //     print(" OFF");
        // else
        //     print(" ON");

        if (PINC & (1 << PC2) && (aim_x > target_x - target_radius && aim_x < target_x + target_radius && aim_y > target_y - target_radius && aim_y < target_y + target_radius)) {
            target_x = (rand() % (WIDTH - 20)) + 10;
            target_y = (rand() % (HEIGHT - 20)) + 10;
            points++;
        }
    }
    return 0;
}

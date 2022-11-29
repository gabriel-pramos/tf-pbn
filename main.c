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

uint16_t aim_x = WIDTH / 2;
uint16_t aim_y = HEIGHT / 2;

uint16_t target_x = WIDTH / 2;
uint16_t target_y = HEIGHT / 2;
uint16_t target_radius = 0;
uint16_t target_life = 0;

uint16_t points = 0;
uint16_t targets = 11;

void draw_start();
void draw_end();
void draw();
void draw_points();
void draw_target();
void draw_aim();
void new_target();

void draw_start()
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(15, 0);
    nokia_lcd_write_string("10 Alvos!", 1);
    nokia_lcd_set_cursor(0, 20);
    nokia_lcd_write_string("Controles:", 1);
    nokia_lcd_set_cursor(0, 30);
    nokia_lcd_write_string("g para comecar", 1);
    nokia_lcd_set_cursor(0, 40);
    nokia_lcd_write_string("s para atirar", 1);
    nokia_lcd_render();
}

void draw_end()
{
    nokia_lcd_clear();
    nokia_lcd_set_cursor(30, 10);
    nokia_lcd_write_string("Fim!", 1);
    char sp[10];
    sprintf(sp, "Pontos: %d", points);
    nokia_lcd_set_cursor(10, 25);
    nokia_lcd_write_string(sp, 1);
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

void new_target()
{
    targets--;
    target_x = (rand() % (WIDTH - 24)) + 12;
    target_y = (rand() % (HEIGHT - 24)) + 12;
    target_life = 12000;
}

// Rotina de tratamento da interrupção PCINT1
ISR(PCINT1_vect)
{
    if ((PINC & (1 << PC2)) && targets > 0) {
        if (target_x - target_radius <= aim_x && aim_x <= target_x + target_radius && target_y - target_radius <= aim_y && aim_y <= target_y + target_radius) {
            points += target_radius;
            new_target();
        }
    }
    _delay_ms(1);
}

int main()
{
    // set PC2 and PC3 as input
    DDRC &= ~(1 << DDC2);
    DDRC &= ~(1 << DDC3);

    // habilita vetor de interrupção para PB1 ... PB5 e PC0 ... PC5
    PCICR |= (1 << PCIE1);
    PCICR |= (1 << PCIE2);

    // habilita interrupção para PC2
    PCMSK1 |= (1 << PCINT10);

    sei();

    USART_Init();
    adc_init();

    nokia_lcd_init();

    draw_start();
    int r = 0;
    while (!(PINC & (1 << PC3)))
        r++;
    srand(r);

    new_target();

    for (;;) {
        if (targets == 0) {
            draw_end();
        } else {
            draw();
            adc_set_channel(0);
            float x = adc_read();
            adc_set_channel(1);
            float y = adc_read();

            if (aim_x + ((x - 511) / 100) > 5 && aim_x + ((x - 511) / 100) < WIDTH - 5)
                aim_x += (x - 511) / 100;
            if (aim_y - ((y - 511) / 100) > 5 && aim_y - ((y - 511) / 100) < HEIGHT - 5)
                aim_y -= (y - 511) / 100;

            target_radius = target_life / 1000;
            if (target_radius <= 1)
                new_target();
            target_life -= 50;
            // _delay_ms(1);
        }
    }

    return 0;
}

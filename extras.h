/*
 * This file is part of i3lock-extended
 * Copyright (C) 2020 Simeon Simeonov

 * i3lock-extended is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * i3lock-extended is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef EXTRAS_H
#define EXTRAS_H

#include <time.h>


#define I3LOCK_SCALE_LED_LENGTH 60
#define I3LOCK_SCALE_LED_SPACING 30


typedef enum {

    I3LOCK_ALIGN_LEFT = 0,
    I3LOCK_ALIGN_CENTER,
    I3LOCK_ALIGN_RIGHT

} i3lock_horizontal_alignment;


typedef enum {

    I3LOCK_ALIGN_TOP = 0,
    I3LOCK_ALIGN_MIDDLE,
    I3LOCK_ALIGN_BOTTOM

} i3lock_vertical_alignment;


typedef struct i3lock_digital_clock_t_ {

    int text_size;
    char color[7];
    char *template;
    i3lock_horizontal_alignment horizontal_alignment;
    i3lock_vertical_alignment vertical_alignment;

} i3lock_digital_clock_t;


typedef struct i3lock_elapsed_time_t_ {

    int text_size;
    char color[7];
    time_t start_time;
    i3lock_horizontal_alignment horizontal_alignment;
    i3lock_vertical_alignment vertical_alignment;

} i3lock_elapsed_time_t;


typedef struct i3lock_led_clock_t_ {

    char led_on_color[7];
    char led_off_color[7];
    char led_border_color[7];
    i3lock_horizontal_alignment horizontal_alignment;
    i3lock_vertical_alignment vertical_alignment;

} i3lock_led_clock_t;


void i3lock_draw_digital_clock(cairo_t *cr,
                               const i3lock_digital_clock_t *dc,
                               const uint32_t *resolution,
                               const struct tm *brokentime);


void i3lock_draw_elapsed_time(cairo_t *cr,
                              const i3lock_elapsed_time_t *et,
                              const uint32_t *resolution,
                              const time_t *now);

void i3lock_draw_led_clock(cairo_t *cr,
                           const i3lock_led_clock_t *lc,
                           const uint32_t *resolution,
                           const struct tm *brokentime);


void i3lock_draw_string(cairo_t *cr,
                        const char *formatted_string,
                        const i3lock_elapsed_time_t *et,
                        const uint32_t *resolution);

char * i3lock_format_elapsed_time(char *to,
                                  const char *template,
                                  int max_size,
                                  int seconds);
#endif

/*
 * This file is part of i3lock-extended
 * Copyright (C) 2020-2023 Simeon Simeonov

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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cairo.h>

#include <xcb/xcb.h>
#include <xcb/xcb_aux.h>
#include <xcb/randr.h>

#include "randr.h"
#include "extras.h"


extern int failed_attempts;


static void i3lock_color_to_array(double *rgb16, const char *color) {

    char strgroups[3][3] = {{color[0], color[1], '\0'},
                            {color[2], color[3], '\0'},
                            {color[4], color[5], '\0'}};
    *rgb16 = strtol(strgroups[0], NULL, 16) / 255.0;
    *(rgb16 + 1) = strtol(strgroups[1], NULL, 16) / 255.0;
    *(rgb16 + 2) = strtol(strgroups[2], NULL, 16) / 255.0;

}


static void i3lock_fetch_digital_clock_xy(int *x,
                                          int *y,
                                          const i3lock_digital_clock_t *dc,
                                          const uint32_t *resolution,
                                          int text_length) {

    /* x */
    if (dc->horizontal_alignment == I3LOCK_ALIGN_CENTER) {
        *x = resolution[0] / 2 - text_length * dc->text_size / 2;
    } else if (dc->horizontal_alignment == I3LOCK_ALIGN_RIGHT) {
        *x = resolution[0] - resolution[0] / 10 - text_length * dc->text_size;
    } else {
        /* left */
        *x = resolution[0] / 10; /* 10% */
    }

    /* y */
    if (dc->vertical_alignment == I3LOCK_ALIGN_MIDDLE) {
        *y = resolution[1] / 2 - dc->text_size / 2;
    } else if (dc->vertical_alignment == I3LOCK_ALIGN_BOTTOM) {
        *y = resolution[1] - resolution[1] / 10 - dc->text_size;
    } else {
        /* top */
        *y = resolution[1] / 10; /* 10% */
    }
        
}


static void i3lock_fetch_led_clock_xy(int *x,
                                       int *y,
                                       const i3lock_led_clock_t *lc,
                                       const uint32_t *resolution) {
    int clock_length, clock_height;

    /* x */
    clock_length = 6 * resolution[1] / I3LOCK_SCALE_LED_SPACING + \
        5 * resolution[1] / I3LOCK_SCALE_LED_LENGTH;
    if (lc->horizontal_alignment == I3LOCK_ALIGN_CENTER) {
        *x = resolution[0] / 2 - clock_length / 2;
    } else if (lc->horizontal_alignment == I3LOCK_ALIGN_RIGHT) {
        *x = resolution[0] - resolution[0] / 10 - clock_length;
    } else {
        /* left */
        *x = resolution[0] / 10; /* 10% */
    }

    /* y */
    clock_height = 3 * resolution[1] / I3LOCK_SCALE_LED_SPACING + \
        2 * resolution[1] / I3LOCK_SCALE_LED_LENGTH;
    if (lc->vertical_alignment == I3LOCK_ALIGN_MIDDLE) {
        *y = resolution[1] / 2 - clock_height / 2;
    } else if (lc->vertical_alignment == I3LOCK_ALIGN_BOTTOM) {
        *y = resolution[1] - resolution[1] / 10 - clock_height;
    } else {
        /* top */
        *y = resolution[1] / 10; /* 10% */
    }
        
}


static void i3lock_draw_led(cairo_t *cr,
                            double x,
                            double y,
                            double length,
                            int type,
                            const i3lock_led_clock_t *lc) {
    double border_color[3], face_color[3];
    i3lock_color_to_array(border_color, lc->led_border_color);
    if (type) {
        /* LED on */
        i3lock_color_to_array(face_color, lc->led_on_color);
    } else {
        i3lock_color_to_array(face_color, lc->led_off_color);
    }
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, x, y, length, length);
    cairo_set_source_rgb(cr, face_color[0], face_color[1], face_color[2]);
    cairo_fill(cr);
    cairo_rectangle(cr, x, y, length, length);
    cairo_set_source_rgb(cr, border_color[0], border_color[1], border_color[2]);
    cairo_stroke(cr);
}


static char * i3lock_replace_str(char *to,
                                 const char *from,
                                 const char *old_substr,
                                 const char *new_substr,
                                 size_t max_length) {

    int offset; /* the offset in the tmp_str after each match */

    /* the offset in from needed to pick up the right side after a match */
    int from_offset;

    int from_len = strlen(from);
    int new_substr_len = strlen(new_substr);
    int old_substr_len = strlen(old_substr);
    int len_diff = old_substr_len - new_substr_len;
    size_t free_space = max_length - 1;
    char tmp_str[max_length];
    char *tmp_str_ptr;

    /* reserve the last byte for '\0' in case of clipping */
    strncpy(tmp_str, from, free_space);

    for (int i = 0; ; ++i) {
        tmp_str_ptr = strstr(tmp_str, old_substr);
        if (tmp_str_ptr == NULL) {
            /* no more instances of old_substr */
            break;
        }

        offset = tmp_str_ptr - tmp_str;
        free_space -= offset;
        if (free_space <= new_substr_len) {
            strncpy(tmp_str_ptr, new_substr, free_space);
            break;
        }
        strncpy(tmp_str_ptr, new_substr, new_substr_len);
        free_space -= new_substr_len;
        /* tmp_str offset -> from_offset mapping */
        from_offset = offset + old_substr_len + i * len_diff;
        if (free_space <= from_len - from_offset) {
            strncpy(tmp_str_ptr + new_substr_len, from + from_offset, free_space);
            break;
        }
        strncpy(tmp_str_ptr + new_substr_len, from + from_offset, from_len - from_offset + 1);

    }

    strncpy(to, tmp_str, max_length - 1);
    *(to + (max_length - 1)) = '\0';

    return to;

}


void i3lock_draw_digital_clock(cairo_t *cr,
                               const i3lock_digital_clock_t *dc,
                               const uint32_t *resolution,
                               const struct tm *brokentime) {
    double rgb16[3];
    char current_time_str[128];
    int x, y;
    strftime(current_time_str,
             128,
             dc->template,
             brokentime);
    i3lock_color_to_array(rgb16, dc->color);
    cairo_set_source_rgb(cr,
                         rgb16[0],
                         rgb16[1],
                         rgb16[2]);
    cairo_set_font_size(cr, dc->text_size);
    if (xr_screens > 0) {
        /* xinerama */
        int screen;
        uint32_t screen_resolution[2];
        for (screen = 0; screen < xr_screens; ++screen) {
            screen_resolution[0] = xr_resolutions[screen].width;
            screen_resolution[1] = xr_resolutions[screen].height;
            i3lock_fetch_digital_clock_xy(&x,
                                          &y,
                                          dc,
                                          screen_resolution,
                                          strlen(current_time_str));
            cairo_move_to(cr,
                          x + xr_resolutions[screen].x,
                          y + xr_resolutions[screen].y);
            cairo_show_text(cr, current_time_str);
        }
        return;
    }

    i3lock_fetch_digital_clock_xy(&x,
                                  &y,
                                  dc,
                                  resolution,
                                  strlen(current_time_str));
    /* printf("%dx%d\n", *resolution, *(resolution + 1)); */
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, current_time_str);

}


void i3lock_draw_string(cairo_t *cr,
                        const char *formatted_string,
                        const i3lock_elapsed_time_t *et,
                        const uint32_t *resolution) {
    double rgb16[3];
    int x, y;

    /* create a dummy i3lock_digital_clock_t structure in order to reuse
       i3lock_fetch_digital_clock_xy */
    i3lock_digital_clock_t dc = {et->text_size,
                                 "",
                                 "",
                                 et->horizontal_alignment,
                                 et->vertical_alignment};
    i3lock_color_to_array(rgb16, et->color);
    cairo_set_source_rgb(cr,
                         rgb16[0],
                         rgb16[1],
                         rgb16[2]);
    cairo_set_font_size(cr, et->text_size);
    if (xr_screens > 0) {
        /* xinerama */
        int screen;
        uint32_t screen_resolution[2];
        for (screen = 0; screen < xr_screens; ++screen) {
            screen_resolution[0] = xr_resolutions[screen].width;
            screen_resolution[1] = xr_resolutions[screen].height;
            i3lock_fetch_digital_clock_xy(&x,
                                          &y,
                                          &dc,
                                          screen_resolution,
                                          strlen(formatted_string));
            cairo_move_to(cr,
                          x + xr_resolutions[screen].x,
                          y + xr_resolutions[screen].y);
            cairo_show_text(cr, formatted_string);
        }
        return;
    }

    i3lock_fetch_digital_clock_xy(&x,
                                  &y,
                                  &dc,
                                  resolution,
                                  strlen(formatted_string));
    /* printf("%dx%d\n", *resolution, *(resolution + 1)); */
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, formatted_string);

}


void i3lock_draw_elapsed_time(cairo_t *cr,
                              const i3lock_elapsed_time_t *et,
                              const uint32_t *resolution,
                              const time_t *now) {
    double rgb16[3];
    char elapsed_time_str[128];
    int x, y, seconds, minutes, hours, diff_seconds;

    /* create a dummy i3lock_digital_clock_t structure in ordet to reuse
       i3lock_fetch_digital_clock_xy */
    i3lock_digital_clock_t dc = {et->text_size,
                                 "",
                                 "",
                                 et->horizontal_alignment,
                                 et->vertical_alignment};

    diff_seconds = (int) difftime(*now, et->start_time);
    hours = diff_seconds / 3600;
    minutes = (diff_seconds - hours * 3600) / 60;
    seconds = diff_seconds - hours * 3600 - minutes * 60;
    if (hours) {
        snprintf(elapsed_time_str,
                 128,
                 "%d:%02d:%02d",
                 hours,
                 minutes,
                 seconds);
    } else {
        snprintf(elapsed_time_str, 128, "%02d:%02d", minutes, seconds);
    }
    if (failed_attempts) {
        char failed_attempts_str[16]; /* ... that many attempts */
        snprintf(failed_attempts_str,
                 16,
                 " (%d)",
                 failed_attempts);
        strncat(elapsed_time_str, failed_attempts_str, 16);
    }
    i3lock_color_to_array(rgb16, et->color);
    cairo_set_source_rgb(cr,
                         rgb16[0],
                         rgb16[1],
                         rgb16[2]);
    cairo_set_font_size(cr, et->text_size);
    if (xr_screens > 0) {
        /* xinerama */
        int screen;
        uint32_t screen_resolution[2];
        for (screen = 0; screen < xr_screens; ++screen) {
            screen_resolution[0] = xr_resolutions[screen].width;
            screen_resolution[1] = xr_resolutions[screen].height;
            i3lock_fetch_digital_clock_xy(&x,
                                          &y,
                                          &dc,
                                          screen_resolution,
                                          strlen(elapsed_time_str));
            cairo_move_to(cr,
                          x + xr_resolutions[screen].x,
                          y + xr_resolutions[screen].y);
            cairo_show_text(cr, elapsed_time_str);
        }
        return;
    }

    i3lock_fetch_digital_clock_xy(&x,
                                  &y,
                                  &dc,
                                  resolution,
                                  strlen(elapsed_time_str));
    /* printf("%dx%d\n", *resolution, *(resolution + 1)); */
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, elapsed_time_str);

}


void i3lock_draw_led_clock(cairo_t *cr,
                           const i3lock_led_clock_t *lc,
                           const uint32_t *resolution,
                           const struct tm *brokentime) {
    int side_length, space_length, base_x, base_y;
    int base_x_h; /* used to make the code more readable */
    if (xr_screens > 0) {
        /* xinerama */
        int screen;
        uint32_t screen_resolution[2];
        for (screen = 0; screen < xr_screens; ++screen) {
            screen_resolution[0] = xr_resolutions[screen].width;
            screen_resolution[1] = xr_resolutions[screen].height;
            side_length = xr_resolutions[screen].height / I3LOCK_SCALE_LED_SPACING;
            space_length = xr_resolutions[screen].height / I3LOCK_SCALE_LED_LENGTH;
            i3lock_fetch_led_clock_xy(&base_x,
                                      &base_y,
                                      lc,
                                      screen_resolution);
            base_x += xr_resolutions[screen].x;
            base_y += xr_resolutions[screen].y;
            base_x_h = base_x + space_length / 2 + side_length / 2;

            /* hours */
            i3lock_draw_led(cr,
                            base_x_h,
                            base_y,
                            side_length,
                            brokentime->tm_hour & 16,
                            lc);
            i3lock_draw_led(cr,
                            base_x_h + space_length + side_length,
                            base_y,
                            side_length,
                            brokentime->tm_hour & 8,
                            lc);
            i3lock_draw_led(cr,
                            base_x_h + 2 * (space_length + side_length),
                            base_y,
                            side_length,
                            brokentime->tm_hour & 4,
                            lc);
            i3lock_draw_led(cr,
                            base_x_h + 3 * (space_length + side_length),
                            base_y,
                            side_length,
                            brokentime->tm_hour & 2,
                            lc);
            i3lock_draw_led(cr,
                            base_x_h + 4 * (space_length + side_length),
                            base_y,
                            side_length,
                            brokentime->tm_hour & 1,
                            lc);

            /* minutes */
            i3lock_draw_led(cr,
                            base_x,
                            base_y + side_length + space_length,
                            side_length,
                            brokentime->tm_min & 32,
                            lc);
            i3lock_draw_led(cr,
                            base_x + side_length + space_length,
                            base_y + side_length + space_length,
                            side_length,
                            brokentime->tm_min & 16,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 2 * (side_length + space_length),
                            base_y + side_length + space_length,
                            side_length,
                            brokentime->tm_min & 8,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 3 * (side_length + space_length),
                            base_y + side_length + space_length,
                            side_length,
                            brokentime->tm_min & 4,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 4 * (side_length + space_length),
                            base_y + side_length + space_length,
                            side_length,
                            brokentime->tm_min & 2,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 5 * (side_length + space_length),
                            base_y + side_length + space_length,
                            side_length,
                            brokentime->tm_min & 1,
                            lc);

            /* seconds */
            i3lock_draw_led(cr,
                            base_x,
                            base_y + 2 * (side_length + space_length),
                            side_length,
                            brokentime->tm_sec & 32,
                            lc);
            i3lock_draw_led(cr,
                            base_x + side_length + space_length,
                            base_y + 2 * (side_length + space_length),
                            side_length,
                            brokentime->tm_sec & 16,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 2 * (side_length + space_length),
                            base_y + 2 * (side_length + space_length),
                            side_length,
                            brokentime->tm_sec & 8,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 3 * (side_length + space_length),
                            base_y + 2 * (side_length + space_length),
                            side_length,
                            brokentime->tm_sec & 4,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 4 * (side_length + space_length),
                            base_y + 2 * (side_length + space_length),
                            side_length,
                            brokentime->tm_sec & 2,
                            lc);
            i3lock_draw_led(cr,
                            base_x + 5 * (side_length + space_length),
                            base_y + 2 * (side_length + space_length),
                            side_length,
                            brokentime->tm_sec & 1,
                            lc);
        }

        return;

    }

}


char * i3lock_format_elapsed_time(char *to,
                                  const char *template,
                                  int max_size,
                                  int seconds) {

    char tmp_str[max_size];
    char int_str[32]; /* more than enough for int */

    if (seconds < 0) {
        if (!(seconds % 3))
            return NULL;
        seconds = 0;
    }

    int days = seconds / 86400;
    int hours = (seconds - days * 86400) / 3600;
    int minutes = (seconds - days * 86400 - hours * 3600) / 60;
    int rest_seconds = seconds - days * 86400 - hours * 3600 - minutes * 60;

    strncpy(tmp_str, template, max_size);
    if (strlen(template) >= max_size) {
        /* do a proper clipping */
        *(tmp_str + (max_size - 1)) = '\0';
    }

    /* days */
    snprintf(int_str, 32, "%d", days);
    i3lock_replace_str(to, tmp_str, "%d", int_str, max_size);
    strcpy(tmp_str, to);

    /* hours */
    snprintf(int_str, 32, "%d", hours);
    i3lock_replace_str(to, tmp_str, "%h", int_str, max_size);
    strcpy(tmp_str, to);

    /* minutes */
    snprintf(int_str, 32, "%d", minutes);
    i3lock_replace_str(to, tmp_str, "%m", int_str, max_size);
    strcpy(tmp_str, to);

    /* rest. seconds */
    snprintf(int_str, 32, "%d", rest_seconds);
    i3lock_replace_str(to, tmp_str, "%s", int_str, max_size);
    strcpy(tmp_str, to);

    /* seconds */
    snprintf(int_str, 32, "%d", seconds);
    i3lock_replace_str(to, tmp_str, "%S", int_str, max_size);

    return to;

}

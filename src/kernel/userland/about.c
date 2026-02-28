// Copyright (c) 2023-2026 Chris (boreddevnl)
// This software is released under the GNU General Public License v3.0. See LICENSE file for details.
// This header needs to maintain in any file it is present in, as per the GPL license terms.
#include "syscall.h"
#include "libui.h"
#include <stddef.h>

static void draw_boredos_logo(ui_window_t win, int x, int y, int scale) {
    static const uint8_t brewos_bmp[] = {
        0,0,1,1,1,0,0,0,0,0,0,1,1,1,0,0,
        0,1,1,1,1,0,0,0,0,0,0,1,1,1,1,0, 
        1,1,1,1,1,0,0,0,0,0,0,1,1,1,1,1, 
        1,1,1,1,2,2,2,2,2,2,2,2,1,1,1,1, 
        1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1, 
        1,1,2,2,2,1,1,2,2,1,1,2,2,2,1,1, 
        1,1,2,2,1,1,1,1,1,1,1,1,2,2,1,1, 
        1,1,2,2,1,1,1,1,1,1,1,1,2,2,1,1,
        1,1,2,2,1,1,1,1,1,1,1,1,2,2,1,1,
        1,1,2,2,2,1,1,2,2,1,1,2,2,2,1,1, 
        1,1,2,2,2,2,2,1,1,2,2,2,2,2,1,1, 
        1,1,2,2,2,2,2,2,2,2,2,2,2,2,1,1, 
        1,1,1,2,2,2,2,2,2,2,2,2,2,1,1,1,
        0,1,1,1,2,2,2,2,2,2,2,2,1,1,1,0, 
        0,0,1,1,1,2,2,2,2,2,2,1,1,1,0,0, 
        0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0 
    };

    for (int r = 0; r < 16; r++) {
        for (int c = 0; c < 16; c++) {
            uint8_t p = brewos_bmp[r * 16 + c];
            if (p == 1) {
                ui_draw_rect(win, x + c * scale, y + r * scale, scale, scale, 0xFF1A1A1A);
            } else if (p == 2) {
                ui_draw_rect(win, x + c * scale, y + r * scale, scale, scale, 0xFFFEFEFE);
            }
        }
    }
}

static void about_paint(ui_window_t win) {
    int w = 185;
    int h = 240;
    
    
    int offset_x = 15;
    int offset_y = 35;
    
    draw_boredos_logo(win, 60, offset_y, 4);
    
    // Version info
    ui_draw_string(win, offset_x, offset_y + 105, "BoredOS 'Panda'", 0xFFFFFFFF);
    ui_draw_string(win, offset_x, offset_y + 120, "BoredOS Version 1.64", 0xFFFFFFFF);
    ui_draw_string(win, offset_x, offset_y + 135, "Kernel Version 3.0.0", 0xFFFFFFFF);
    
    // Copyright
    ui_draw_string(win, offset_x, offset_y + 150, "(C) 2026 boreddevnl.", 0xFFFFFFFF);
    ui_draw_string(win, offset_x, offset_y + 165, "All rights reserved.", 0xFFFFFFFF);
    
    ui_mark_dirty(win, 0, 0, w, h);
}

int main(void) {
    ui_window_t win_about = ui_window_create("About BoredOS", 250, 180, 185, 240);
    
    about_paint(win_about);
    
    gui_event_t ev;
    while (1) {
        if (ui_get_event(win_about, &ev)) {
            if (ev.type == GUI_EVENT_PAINT) {
                about_paint(win_about);
            } else if (ev.type == GUI_EVENT_CLOSE) {
                sys_exit(0);
            }
        } else {
            // Avoid high CPU usage
            for(volatile int i=0; i<10000; i++);
        }
    }
    
    return 0;
}

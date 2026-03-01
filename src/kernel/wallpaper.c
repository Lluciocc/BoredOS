#include "wallpaper.h"
#include "nanojpeg.h"
#include "graphics.h"
#include "fat32.h"
#include "memory_manager.h"
#include "wm.h"
#include "io.h"
#include <stddef.h>

// Static buffer for the current wallpaper (max 1920x1080)
#define MAX_WP_WIDTH 1920
#define MAX_WP_HEIGHT 1080
static uint32_t wp_pixels[MAX_WP_WIDTH * MAX_WP_HEIGHT];
static int wp_width = 0;
static int wp_height = 0;

// Deferred wallpaper action (set from interrupt context, processed in main loop)
static volatile const char *pending_wallpaper_path = NULL;
static char pending_path_buf[256];

// Simple nearest-neighbor scale from decoded RGB to ARGB pixel buffer
static void scale_rgb_to_argb(const unsigned char *rgb, int src_w, int src_h,
                              uint32_t *dst, int dst_w, int dst_h) {
    for (int y = 0; y < dst_h; y++) {
        int src_y = y * src_h / dst_h;
        if (src_y >= src_h) src_y = src_h - 1;
        for (int x = 0; x < dst_w; x++) {
            int src_x = x * src_w / dst_w;
            if (src_x >= src_w) src_x = src_w - 1;
            int idx = (src_y * src_w + src_x) * 3;
            unsigned char r = rgb[idx];
            unsigned char g = rgb[idx + 1];
            unsigned char b = rgb[idx + 2];
            dst[y * dst_w + x] = 0xFF000000 | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }
    }
}

// Decode JPEG data from memory and set as wallpaper (MUST be called from non-interrupt context)
static int decode_and_set_wallpaper(const unsigned char *jpg_data, unsigned int jpg_size) {
    njInit();
    nj_result_t result = njDecode(jpg_data, (int)jpg_size);
    if (result != NJ_OK) {
        njDone();
        return 0;
    }

    int img_w = njGetWidth();
    int img_h = njGetHeight();
    unsigned char *rgb = njGetImage();

    if (!rgb || img_w <= 0 || img_h <= 0) {
        njDone();
        return 0;
    }

    // Scale to screen size
    int screen_w = get_screen_width();
    int screen_h = get_screen_height();
    if (screen_w > MAX_WP_WIDTH) screen_w = MAX_WP_WIDTH;
    if (screen_h > MAX_WP_HEIGHT) screen_h = MAX_WP_HEIGHT;

    scale_rgb_to_argb(rgb, img_w, img_h, wp_pixels, screen_w, screen_h);
    wp_width = screen_w;
    wp_height = screen_h;

    njDone();

    graphics_set_bg_image(wp_pixels, wp_width, wp_height);
    return 1;
}

// Simple serial output for debugging (COM1 = 0x3F8)
static void serial_char(char c) {
    while (!(inb(0x3F8 + 5) & 0x20));  // Wait for transmit ready
    outb(0x3F8, c);
}

static void serial_str(const char *s) {
    while (*s) serial_char(*s++);
}

static void serial_num(int n) {
    if (n < 0) { serial_char('-'); n = -n; }
    if (n >= 10) serial_num(n / 10);
    serial_char('0' + (n % 10));
}

// Request wallpaper change by file path (safe to call from interrupt context)
void wallpaper_request_set_from_file(const char *path) {
    // Copy path to buffer
    int i = 0;
    while (path[i] && i < 255) {
        pending_path_buf[i] = path[i];
        i++;
    }
    pending_path_buf[i] = 0;
    pending_wallpaper_path = pending_path_buf;
}

// Process deferred wallpaper actions (called from main loop, NOT interrupt context)
void wallpaper_process_pending(void) {
    if (pending_wallpaper_path) {
        const char *path = (const char *)pending_wallpaper_path;
        pending_wallpaper_path = NULL;

        serial_str("[WP] Processing wallpaper: ");
        serial_str(path);
        serial_str("\n");

        // Read file from filesystem
        FAT32_FileHandle *fh = fat32_open(path, "r");
        if (fh) {
            uint32_t file_size = fh->size;
            if (file_size > 0 && file_size <= 4 * 1024 * 1024) {
                unsigned char *buf = (unsigned char*)kmalloc(file_size);
                if (buf) {
                    int total_read = 0;
                    while (total_read < (int)file_size) {
                        int chunk = fat32_read(fh, buf + total_read, (int)file_size - total_read);
                        if (chunk <= 0) break;
                        total_read += chunk;
                    }
                    fat32_close(fh);

                    if (total_read > 0) {
                        decode_and_set_wallpaper(buf, (unsigned int)total_read);
                        wm_refresh();
                    }
                    kfree(buf);
                } else {
                    fat32_close(fh);
                }
            } else {
                fat32_close(fh);
            }
        }
    }
}

uint32_t* wallpaper_get_pixels(void) { return wp_pixels; }
int wallpaper_get_width(void) { return wp_width; }
int wallpaper_get_height(void) { return wp_height; }

void wallpaper_init(void) {
    // We expect Limine modules to have been copied to /Library/images/Wallpapers/ by main.c
    // Set a default wallpaper if one exists
    if (fat32_exists("/Library/images/Wallpapers/mountain.jpg")) {
        wallpaper_request_set_from_file("/Library/images/Wallpapers/mountain.jpg");
    } else if (fat32_exists("/Library/images/Wallpapers/moon.jpg")) {
        wallpaper_request_set_from_file("/Library/images/Wallpapers/moon.jpg");
    }
}

#include "print.h"

// Global variables to track cursor position
int current_x = 0;
int current_y = 0;

void print_char(char c) {
    // Handle newline special case
    if (c == '\n') {
        current_x = 0;
        current_y++;
        asm volatile(
            "movb $0x0E, %%ah\n\t"
            "movb $0x0D, %%al\n\t" // Carriage return
            "int $0x10\n\t"
            "movb $0x0E, %%ah\n\t"
            "movb $0x0A, %%al\n\t" // Line feed
            "int $0x10"
            ::: "ax"
        );
        return;
    }
    
    // Handle carriage return special case
    if (c == '\r') {
        current_x = 0;
        asm volatile(
            "movb $0x0E, %%ah\n\t"
            "movb $0x0D, %%al\n\t"
            "int $0x10"
            ::: "ax"
        );
        return;
    }

    // Normal character printing
    asm volatile(
        "movb %0, %%al\n\t"    // Character in AL
        "movb $0x0E, %%ah\n\t" // Function 0x0E (teletype output)
        "movb $0x00, %%bh\n\t" // Page number 0
        "movb $0x07, %%bl\n\t" // Normal attribute (light gray on black)
        "int $0x10"
        :
        : "q" (c)
        : "ax", "bx"
    );
    
    // Update cursor position
    current_x++;
    if (current_x >= 80) {  // Assume 80 column screen
        current_x = 0;
        current_y++;
    }
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void print_newline() {
    print_char('\r');
    print_char('\n');
}

void set_video_mode() {
    asm volatile (
        "movb $0x00, %%ah\n\t"
        "movb $0x03, %%al\n\t"
        "int $0x10"
        ::: "ax"
    );
    
    // Reset cursor position trackers
    current_x = 0;
    current_y = 0;
}

void clear_screen() {
    asm volatile (
        "movw $0x0003, %%ax\n\t"
        "int $0x10"
        ::: "ax"
    );
    
    // Reset cursor position trackers
    current_x = 0;
    current_y = 0;
}

// Print a string at a specific position (absolute positioning)
void print_string_at(int x, int y, const char* str) {
    set_cursor_pos(x, y);
    print_string(str);
}

// Print a character at a specific position (absolute positioning)
void print_char_at(int x, int y, char c) {
    set_cursor_pos(x, y);
    print_char(c);
}

// Fixed cursor position function for 16-bit mode
void set_cursor_pos(int x, int y) {
    unsigned char row = (unsigned char)y;
    unsigned char col = (unsigned char)x;
    
    current_x = x;
    current_y = y;
    
    asm volatile (
        "movb $0x02, %%ah\n\t"
        "movb $0x00, %%bh\n\t"
        "movb %0, %%dh\n\t"
        "movb %1, %%dl\n\t"
        "int $0x10"
        :
        : "m" (row), "m" (col)
        : "ax", "bx", "dx"
    );
}

// Fixed get cursor position function for 16-bit mode
void get_cursor_pos(int* x, int* y) {
    unsigned char row, col;
    
    asm volatile (
        "movb $0x03, %%ah\n\t"
        "movb $0x00, %%bh\n\t"
        "int $0x10\n\t"
        "movb %%dh, %0\n\t"
        "movb %%dl, %1"
        : "=m" (row), "=m" (col)
        :
        : "ax", "bx", "cx", "dx"
    );
    
    *y = (int)row;
    *x = (int)col;
    
    // Update internal tracking
    current_x = *x;
    current_y = *y;
}
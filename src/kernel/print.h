#ifndef PRINT_H
#define PRINT_H

void print_char(char c);
void print_string(const char* str);
void print_newline();
void set_video_mode();
void clear_screen();
void set_cursor_pos(int x, int y);
void get_cursor_pos(int* x, int* y);
void print_string_at(int x, int y, const char* str);
void print_char_at(int x, int y, char c);

#endif
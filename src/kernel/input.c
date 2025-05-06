#include "input.h"
#include "print.h"

void get_input(char* buffer, int max_length) {
    int index = 0;
    while (1) {
        char c;
        asm volatile (
            "movb $0x00, %%ah\n\t"
            "int $0x16\n\t"
            "movb %%al, %0"
            : "=r" (c)
            :
            : "ax"
        );

        if (c == '\r') {  
            buffer[index] = '\0';
            print_newline();
            return;
        } else if (c == '\b') { 
            if (index > 0) {
                index--;
                print_char('\b');
                print_char(' ');
                print_char('\b');
            }
        } else if (index < max_length - 1) {
            buffer[index++] = c;
            print_char(c);
        }
    }
}
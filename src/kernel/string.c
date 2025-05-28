#include "string.h"
#include <stdint.h>


#define NULL ((void*)0)
int strcmp(const char* s1, const char* s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strncmp(const char* s1, const char* s2, int n) {
    while (n && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
        n--;
    }
    if (n == 0) return 0;
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

int strlen(const char* str) {
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    return len;
}

char *strchr(const char* str, char c) {
    while (*str) {
        if (*str == c) {
            return (char*)str;
        }
        str++;
    }
    return NULL;
}

/* Custom implementation of memset */
void* memset(void* dest, int value, uint32_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    while (count--) {
        *ptr++ = (unsigned char)value;
    }
    return dest;
}

/* Custom implementation of strcpy */
char* strcpy(char* dest, const char* src) {
    char* original = dest;
    while ((*dest++ = *src++));
    return original;
}

char *strcat(char* dest, const char* src) {
    char* original = dest;
    while (*dest) {
        dest++;
    }
    while ((*dest++ = *src++));
    return original;
}
/* Custom implementation of memcpy */
void* memcpy(void* dest, const void* src, uint32_t count) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    while (count--) {
        *d++ = *s++;
    }
    return dest;
}

/* Custom implementation of int_to_string */
void int_to_string(int value, char* str) {
    char buffer[12]; // Enough for 32-bit integers
    int i = 0;
    int is_negative = 0;

    if (value < 0) {
        is_negative = 1;
        value = -value;
    }

    do {
        buffer[i++] = (value % 10) + '0';
        value /= 10;
    } while (value > 0);

    if (is_negative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    // Reverse the string
    int j;
    for (j = 0; j < i; j++) {
        str[j] = buffer[i - j - 1];
    }
    str[j] = '\0';
}
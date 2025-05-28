#ifndef STRING_H
#define STRING_H
#include <stdint.h>
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, int n);
int strlen(const char* str);
char *strchr(const char* str, char c);
void* memcpy(void* dest, const void* src, uint32_t count);
void int_to_string(int value, char* str) ;
char *strcat(char* dest, const char* src);
char* strcpy(char* dest, const char* src);
void* memset(void* dest, int value, uint32_t count);
#endif
#ifndef BYTEARRAY_H
#define BYTEARRAY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct ByteArray
{
    char max_len;
    char min_len;
    char data_len;
    char alloc_len;

    char *data;
};

void ByteArray_resize(short offset, struct ByteArray *ptr);
void ByteArray_shift(unsigned short index, unsigned short from, struct ByteArray *ptr);
void ByteArray_pop(unsigned short length, unsigned short stop, struct ByteArray *ptr);

struct ByteArray *ByteArray_create(int min_size, int max_size);

int ByteArray_indexOf(char *needle, unsigned short length, struct ByteArray *ptr);
void ByteArray_append(char *data, unsigned short length, struct ByteArray *ptr);
int ByteArray_count(char *needle, unsigned short length, struct ByteArray *ptr);
void ByteArray_remove(unsigned short index, unsigned short length, struct ByteArray *ptr);
void ByteArray_replace(char *needle, char *str, unsigned short needle_len, unsigned short str_len, struct ByteArray *ptr);
bool ByteArray_startsWith(char *str, unsigned short length, struct ByteArray *ptr);
char *ByteArray_substring(unsigned short index, unsigned short length, struct ByteArray *ptr);
void ByteArray_grab(unsigned short index, unsigned short length, struct ByteArray *ptr, struct ByteArray *ptr2);
void ByteArray_grabChars(unsigned short index, unsigned short length, struct ByteArray *ptr, char *ptr2);
int ByteArray_count(char *needle, unsigned short length, struct ByteArray *ptr);
void ByteArray_clear(struct ByteArray *ptr);
#endif // BYTEARRAY_H

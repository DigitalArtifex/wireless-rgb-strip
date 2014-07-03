#include "bytearray.h"

void ByteArray_resize(short offset, struct ByteArray *ptr)
{
    short nSize = (ptr->alloc_len + offset);

    if((nSize >= ptr->min_len) && (nSize <= ptr->max_len))
    {
        ptr->data = (char *) realloc(ptr->data, (nSize + 1) * sizeof(char));
        memset(ptr->data + (ptr->alloc_len + 1), '\0', (nSize - ptr->alloc_len));
        ptr->alloc_len = nSize;
    }

    //Rolling left overflow
    else if((ptr->data_len >= ptr->alloc_len) && (offset > 0))
    {
        ByteArray_shift(0, offset, ptr);
        ptr->data_len -= offset;
    }
}

void ByteArray_shift(unsigned short index, unsigned short from, struct ByteArray *ptr)
{
    memcpy(ptr->data + index, ptr->data + from, (ptr->data_len - (from - index)));
    memset(ptr->data + (ptr->data_len - (from - index)), '\0', (index + from));
}

void ByteArray_pop(unsigned short length, unsigned short stop, struct ByteArray *ptr)
{
    short nSize = (ptr->data_len + length);

    if((nSize >= ptr->min_len) && (nSize <= ptr->max_len))
    {
        ptr->data = (char *) realloc(ptr->data, (nSize + 1) * sizeof(char));
        memset(ptr->data + ptr->data_len, '\0', ((nSize - ptr->alloc_len) + 1));
        ptr->alloc_len = nSize;
    }
    
    int i;
    for(i = 0; (ptr->data_len - i) > stop; i++)
    {
        if(((ptr->data_len + length) - i) <= ptr->alloc_len)
            ptr->data[(((ptr->data_len + length) - 1) - i)] = ptr->data[((ptr->data_len - 1) - i)];
    }
}

struct ByteArray *ByteArray_create(int min_size, int max_size)
{
    struct ByteArray *_new = (struct ByteArray *)malloc((sizeof(struct ByteArray)));
    _new->data_len = 0;
    _new->max_len = max_size;
    _new->min_len = min_size;
    _new->alloc_len = min_size;
    _new->data = (char *) malloc((min_size + 1) * sizeof(char));

    return _new;
}

int ByteArray_indexOf(char *needle, unsigned short length, struct ByteArray *ptr)
{
    length = (length > 0) ? length : strlen(needle);
    
    int i;
    for(i = 0; i < ptr->data_len; i++)
    {
        if(ptr->data[i] == needle[0])
        {
            bool found = true;
            
            if(length > 1)
            {
                int p;
                for(p = 1; p < length; p++)
                {
                    if(ptr->data[(i + p)] != needle[p])
                    {
                        found = false;
                        break;
                    }
                }
            }

            if(found)
                return i;
        }
    }

    return -1;
}

void ByteArray_append(char *data, unsigned short length, struct ByteArray *ptr)
{
    length = (length > 0) ? length : strlen(data);

    if((ptr->data_len + length) > ptr->alloc_len)
        ByteArray_resize(((ptr->data_len + length) - ptr->alloc_len), ptr);

    memcpy(ptr->data + ptr->data_len, data, length);
    ptr->data_len += length;
}

int ByteArray_count(char *needle, unsigned short length, struct ByteArray *ptr)
{
    length = (length > 0) ? length : strlen(needle);
    int times = 0;
    
    int i;
    for(i = 0; i < ptr->data_len; i++)
    {
        if(ptr->data[i] == needle[0])
        {
            bool found = true;
    
            int p;
            for(p = 1; p < length; p++)
            {
                if(ptr->data[(i + p)] != needle[p])
                {
                    found = false;
                    break;
                }
            }

            if(found)
                times++;
        }
    }

    return times;
}

void ByteArray_remove(unsigned short index, unsigned short length, struct ByteArray *ptr)
{    
    int i;
    for(i = index; (i < length) && (i < ptr->data_len); i++)
        ptr->data[i] = 0;

    ByteArray_shift(index, (index + length), ptr);
    ByteArray_resize((length * -1), ptr);

    ptr->data_len -= length;
}

void ByteArray_replace(char *needle, char *str, unsigned short needle_len, unsigned short str_len, struct ByteArray *ptr)
{
    int index = ByteArray_indexOf(needle, needle_len, ptr);

    if(index >= 0)
    {
        needle_len = ((needle_len > 0) ? needle_len : strlen((char *)needle));
        str_len = ((str_len > 0) ? str_len : strlen((char *)str));

        if(str_len > needle_len)
            ByteArray_pop((str_len - needle_len), index, ptr);
        else
            ByteArray_shift((index + str_len), (index + needle_len), ptr);

        ptr->data_len += (str_len - needle_len);
        memcpy(ptr->data + index, str, str_len);
    }
}

bool ByteArray_startsWith(char *str, unsigned short length, struct ByteArray *ptr)
{
    length = (length > 0) ? length : strlen((char *)str);
    
    int i;
    for(i = 0; i < length; i++)
    {
        if(ptr->data[i] == str[0])
        {    
            int p;
            for(p = 1; p < length; p++)
            {
                if(ptr->data[(i + p)] != str[p])
                    return false;
            }

            return true;
        }
    }

    return false;
}

char *ByteArray_substring(unsigned short index, unsigned short length, struct ByteArray *ptr)
{
    char *retVal = (char *)malloc((length + 1) * sizeof(char));
    memcpy(retVal, ptr->data + index, length);
    retVal[length] = '\0';

    return retVal;
}

void ByteArray_grab(unsigned short index, unsigned short length, struct ByteArray *ptr, struct ByteArray *ptr2)
{
    memset(ptr2->data, '\0', ptr2->alloc_len);
    memcpy(ptr2->data, ptr->data + index, length);
    ptr2->data_len = (char)length;
    ByteArray_remove(index, length, ptr);
}

void ByteArray_grabChars(unsigned short index, unsigned short length, struct ByteArray *ptr, char *ptr2)
{
    memset(ptr2, '\0', length + 1);
    memcpy(ptr2, ptr->data + index, length);
    ByteArray_remove(index, length, ptr);
}

void ByteArray_clear(struct ByteArray *ptr)
{
    memset(ptr->data, '\0', ptr->alloc_len);
    ptr->data_len = 0;
}

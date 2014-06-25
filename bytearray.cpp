#include "bytearray.h"

DigitalArtifex::ByteArray::ByteArray()
    {
        //track allocation size and limits
        this->length = 0;
        this->max_length = 1024;
        this->min_length = 0;

        //track populated bytes
        this->data_length = 0;

        *(this->data) = *(char *) malloc(1 * sizeof(char));
        memset(this->data, '\0', 1);
    }

    DigitalArtifex::ByteArray::ByteArray(unsigned short length)
    {
        //track allocation length and limits
        this->length = length;
        this->max_length = length;
        this->min_length = length;

        //track populated bytes
        this->data_length = 0;

        this->data = (char *) malloc((length + 1) * sizeof(char));
        memset(this->data, '\0', (this->length + 1));
    }

    DigitalArtifex::ByteArray::ByteArray(unsigned short max_size, unsigned short min_size)
    {
        this->length = 0;
        this->max_length = max_size;
        this->min_length = min_size;
        this->data_length = 0;

        *(this->data) = *(char *) malloc((this->length + 1) * sizeof(char));
        memset(this->data, '\0', (this->length + 1));
    }

    DigitalArtifex::ByteArray::~ByteArray()
    {
        //free(this->data);
    }

    void DigitalArtifex::ByteArray::append(char c)
    {
        //Attempt to allocate a single byte, plus null term byte
        if(this->data_length == this->length)
            this->BufferResize(1);

        this->data[this->data_length++] = c;
    }

    void DigitalArtifex::ByteArray::append(char *str, unsigned short length)
    {
        length = (length > 0) ? length : strlen(str);

        if((this->data_length + length) > this->length)
            this->BufferResize(((this->data_length + length) - this->length));

        memcpy(this->data + this->data_length, str, length);
        this->data_length += length;
    }

    void DigitalArtifex::ByteArray::BufferPop(unsigned short length, unsigned short stop)
    {
        short nSize = (this->length + length);

        if((nSize >= this->min_length) && (nSize <= this->max_length))
        {
            (char **) realloc(this->data, (nSize + 1) * sizeof(char *));
            memset(this->data + this->length, '\0', ((nSize - this->length) + 1));
            this->length = nSize;
        }

        for(int i = 0; (this->data_length - i) > stop; i++)
        {
            if(((this->data_length + length) - i) <= this->length)
                this->data[(((this->data_length + length) - 1) - i)] = this->data[((this->data_length - 1) - i)];
        }
    }

    void DigitalArtifex::ByteArray::BufferResize(short offset)
    {
        short nSize = (this->length + offset);

        if((nSize >= this->min_length) && (nSize <= this->max_length))
        {
            (char **) realloc(this->data, (nSize + 1) * sizeof(char *));
            memset(this->data + (this->length + 1), '\0', (nSize - this->length));
            this->length = nSize;
        }

        //Rolling left overflow
        else if((this->data_length >= this->length) && (offset > 0))
        {
            this->BufferShift(0, offset);
            this->data_length -= offset;
        }
    }

    void DigitalArtifex::ByteArray::BufferShift(unsigned short index, unsigned short from)
    {
        memcpy(this->data + index, this->data + from, (this->data_length - (from - index)));
        memset(this->data + (this->data_length - (from - index)), '\0', (index + from));
    }

    bool DigitalArtifex::ByteArray::contains(char c)
    {
        return (this->indexOf(c) >= 0);
    }

    void DigitalArtifex::ByteArray::clear()
    {
        this->remove(0,this->data_length);
    }

    bool DigitalArtifex::ByteArray::contains(char *needle, unsigned short length)
    {
        return (this->indexOf(needle, length) >= 0);
    }

    unsigned short DigitalArtifex::ByteArray::count(char needle)
    {
        int times = 0;

        for(int i = 0; i < this->data_length; i++)
        {
            if(this->data[i] == needle)
                times++;
        }

        return times;
    }

    unsigned short DigitalArtifex::ByteArray::count(char *needle, unsigned short length)
    {
        length = (length > 0) ? length : strlen(needle);
        int times = 0;

        for(int i = 0; i < this->data_length; i++)
        {
            if(this->data[i] == needle[0])
            {
                bool found = true;

                for(int p = 1; p < length; p++)
                {
                    if(this->data[(i + p)] != needle[p])
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

    char *DigitalArtifex::ByteArray::c_str()
    {
        return (char *)(this->data);
    }

    char *DigitalArtifex::ByteArray::grab(unsigned short index, unsigned short length)
    {
        char* retVal = (char*)malloc((length + 1) * sizeof(char));
        memcpy(retVal, this->data + index, length);
        retVal[length] = '\0';
        this->remove(index, length);

        return retVal;
    }

    short DigitalArtifex::ByteArray::indexOf(char needle)
    {
        for(int i = 0; i < this->data_length; i++)
        {
            if(this->data[i] == needle)
                return i;
        }

        return -1;
    }

    short DigitalArtifex::ByteArray::indexOf(char *needle, unsigned short length)
    {
        length = (length > 0) ? length : strlen(needle);

        for(int i = 0; i < this->data_length; i++)
        {
            if(this->data[i] == needle[0])
            {
                bool found = true;

                for(int p = 1; p < length; p++)
                {
                    if(this->data[(i + p)] != needle[p])
                    {
                        found = false;
                        break;
                    }
                }

                if(found)
                    return i;
            }
        }

        return -1;
    }

    void DigitalArtifex::ByteArray::remove(unsigned short index, unsigned short length)
    {
        for(int i = index; (i < length) && (i < this->data_length); i++)
            this->data[i] = 0;

        this->BufferShift(index, (index + length));
        this->BufferResize((length * -1));

        this->data_length -= length;
    }

    void DigitalArtifex::ByteArray::replace(char *needle, char *str, unsigned short needle_len, unsigned short str_len)
    {
        unsigned short index = this->indexOf(needle, needle_len);

        if(index >= 0)
        {
            needle_len = ((needle_len > 0) ? needle_len : strlen((char *)needle));
            str_len = ((str_len > 0) ? str_len : strlen((char *)str));

            if(str_len > needle_len)
                this->BufferPop((str_len - needle_len), index);
            else
                this->BufferShift((index + str_len), (index + needle_len));

            this->data_length += (str_len - needle_len);
            memcpy(this->data + index, str, str_len);
        }
    }

    unsigned short DigitalArtifex::ByteArray::size()
    {
        return this->data_length;
    }

    unsigned short DigitalArtifex::ByteArray::split(char **buffer, char needle, bool retainRemainder)
    {
        unsigned short count = this->count(needle);
        buffer = new char *[(count + retainRemainder ? 0 : 1)];

        for(int i = 0; i < count; i++)
        {
            short index = this->indexOf(needle);
            buffer[i] = (char*)malloc((index + 1) * sizeof(char*));
            buffer[i] = this->grab(0, index);
            buffer[index + 1] = '\0';
            this->remove(0,1);
        }

        if(!retainRemainder && (count > 0))
           buffer[count++] = this->grab(0, this->data_length);

        return count;
    }

    unsigned short DigitalArtifex::ByteArray::split(char **buffer, char *needle, unsigned short length, bool retainRemainder)
    {
        unsigned short count = this->count(needle, length);
        buffer = new char *[(count + retainRemainder ? 0 : 1)];
        length = (length > 0) ? length : strlen(needle);

        for(int i = 0; i < count; i++)
        {
            buffer[i] = this->grab(0, this->indexOf(needle, length));
            this->remove(0,length);
        }

        if(!retainRemainder)
           buffer[count++] = this->grab(0, this->data_length);

        return count;
    }

    bool DigitalArtifex::ByteArray::startsWith(char *str, unsigned short length)
    {
        length = (length > 0) ? length : strlen((char*)str);

        for(int i = 0; i < length; i++)
        {
            if(this->data[i] == str[0])
            {
                for(int p = 1; p < length; p++)
                {
                    if(this->data[(i + p)] != str[p])
                        return false;
                }

                return true;
            }
        }

        return false;
    }

    char *DigitalArtifex::ByteArray::substring(unsigned short index, unsigned short length)
    {
        char *retVal = new char[length];

        for(int i = 0; (i < length) && ((i + index) < this->data_length); i++)
            retVal[i] = this->data[(i + index)];

        return retVal;
    }

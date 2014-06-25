#pragma once

#include <Arduino.h>

namespace DigitalArtifex
{
///
/// \brief Provides Arduino with a dynamically or statically allocated buffer, with the option
/// of setting minimum and maximum sizes.
///
/// This class also provides indexing, replacement, splitting and streaming functionality.
    class ByteArray
    {
        char *data;
        unsigned short length;
        unsigned short data_length;
        unsigned short max_length;
        unsigned short min_length;

        void BufferResize(short offset);
        void BufferShift(unsigned short index, unsigned short length);
        void BufferPop(unsigned short length, unsigned short stop = 0);

    public:

        ///
        /// \brief ByteArray with a dynamic buffer size
        ///
        ByteArray();

        ///
        /// \brief ByteArray with a static buffer size
        /// \param length The number of bytes to reserve for the buffer
        ///
        ByteArray(unsigned short length = 0);

        ///
        /// \brief ByteArray with a dynamic buffer size, with limits.
        /// \param max_size The largest size the buffer should be
        /// \param min_size The smallest size the buffer should be
        ///
        ByteArray(unsigned short max_size, unsigned short min_size);

        ~ByteArray();

        ///
        /// \brief Inserts a char to the end of internal buffer
        /// \param c The char to append
        ///
        void append(char c);

        ///
        /// \brief Inserts a char array to the end of the internal buffer
        /// \param str The char array to append
        /// \param length The length of the append
        ///
        /// \note
        /// If length is left, or set, as 0, strlen will be used to resolve its length.
        ///
        void append(char *str, unsigned short length = 0);

        ///
        /// \brief Removes all data from the internal buffer
        ///
        void clear();

        ///
        /// \brief Searches the internal buffer for a char
        /// \param c The char to search for
        /// \return Whether or not the char is present in the internal buffer
        ///
        bool contains(char c);

        ///
        /// \brief Searches the internal buffer for a char array
        /// \param c The char array to search for
        /// \param length The length of the char array to search for
        /// \return Whether or not the char array is present in the internal buffer
        ///
        bool contains(char *needle, unsigned short length = 0);

        ///
        /// \brief Counts the number of instances of the given char
        /// \param needle The char to search for
        /// \return The number of times the char is in the buffer
        ///
        /// \note
        /// If length is left, or set, as 0, strlen will be used to resolve its length.
        ///
        unsigned short count(char needle);

        ///
        /// \brief Counts the number of instances of the given char array
        /// \param needle The char array to search for
        /// \param length The length of the char array to search for
        /// \return The number of times the array is in the buffer
        ///
        /// \note
        /// If length is left, or set, as 0, strlen will be used to resolve its length.
        ///
        unsigned short count(char *needle, unsigned short length = 0);

        ///
        /// \brief Classic style call to get a char string of the internal buffer
        /// \return Internal buffer in chars
        ///
        char *c_str();

        ///
        /// \brief Removes array from internal buffer and returns removed data.
        /// \param index Index to start grabbing
        /// \param length Number of chars to grab
        /// \return a copy of the returned data
        ///
        char *grab(unsigned short index, unsigned short length);

        ///
        /// \brief Find first index of given char
        /// \param needle The char to search for
        /// \return First index of needle. If the array does not contain char, -1 is returned
        ///
        short indexOf(char needle);

        ///
        /// \brief Find first index of given char array
        /// \param needle The char array to search for
        /// \param length The length of the char array to search for
        /// \return First index of needle. If the array does not contain char array, -1 is returned
        ///
        short indexOf(char *needle, unsigned short length = 0);

        ///
        /// \brief Removes array from internal buffer and returns removed data.
        /// \param index Index to start remove
        /// \param length Number of chars to remove
        ///
        void remove(unsigned short index, unsigned short length);

        ///
        /// \brief Replaces char array 'needle' with char array 'str'
        /// \param needle The char array to search for
        /// \param str The char array to replace needle with
        /// \param needle_len The length of needle
        /// \param str_len The length of str
        ///
        /// \note
        /// If either needle_len or str_len is left, or set, as 0, strlen will be used to resolve their length.
        ///
        void replace(char *needle, char *str = "", unsigned short needle_len = 0, unsigned short str_len = 0);

        ///
        /// \brief Returns data length
        /// \return Size of the data in the internal buffer
        ///
        unsigned short size();

        ///
        /// \brief Splits the ByteArray by the specified char and places it into buffer
        /// \param[out] buffer The address to allocate the array to
        /// \param[in] needle The char to split by
        /// \param[in] retainRemainder If set to true, ByteArray will retain the data beyond the last instance of needle
        /// \return The length of the array
        ///
        unsigned short split(char **buffer, char needle, bool retainRemainder = false);

        ///
        /// \brief Splits the ByteArray by the specified char and places it into buffer
        /// \param[out] buffer The address to allocate the array to
        /// \param[in] needle The char array to split by
        /// \param[in] length The length of the char array to split by
        /// \param[in] retainRemainder If set to true, ByteArray will retain the data beyond the last instance of needle
        /// \return The length of the array
        ///
        /// \note
        /// If length is left, or set, as 0, strlen will be used to resolve its length.
        ///
        unsigned short split(char **buffer, char *needle, unsigned short length = 0, bool retainRemainder = false);

        ///
        /// \brief Checks to see if the array begins with a string
        /// \param str The char array to check with the beginning of the array
        /// \param length The length of the array
        /// \return True or false
        ///
        /// \note
        /// If length is left, or set, as 0, strlen will be used to resolve its length.
        ///
        bool startsWith(char *str, unsigned short length = 0);

        ///
        /// \brief Returns a section of the internal buffer in chars
        /// \param index Location of substring start
        /// \param length Number of chars to return
        /// \return The substring
        ///
        char *substring(unsigned short index, unsigned short length);
    };
}

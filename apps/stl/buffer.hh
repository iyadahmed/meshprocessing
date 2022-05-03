#pragma once

#include <cerrno>
#include <cstdio>

#include "importer.hh"

class Buffer
{
private:
    char *m_mem = NULL;
    size_t m_size = 0;
    size_t m_current_location = 0;

public:
    Buffer(size_t size)
    {
        this->m_mem = new char[size];
        this->m_size = size;
        this->m_current_location = 0;
    }

    size_t location() { return m_current_location; }

    size_t size() { return m_size; }

    void printn(size_t n)
    {
        for (int i = 0; (i < n) && ((i + this->m_current_location) < this->m_size);
             i++)
        {
            putchar(this->m_mem[this->m_current_location + i]);
        }
    }

    static Buffer from_file(FILE *file)
    {
        // TODO: error checking for calc file size and fread
        long file_size = calc_file_size(file);
        Buffer buffer(file_size + 1);
        buffer.m_size = fread(buffer.m_mem, 1, file_size, file);
        buffer.m_mem[buffer.m_size] =
            '\0'; // Ensure null termination, for strtof to work
        return buffer;
    }

    void drop_leading_whitespace()
    {
        while (this->m_current_location < m_size)
        {
            if (!isspace(this->m_mem[this->m_current_location]))
            {
                break;
            }
            this->m_current_location++;
        }
    }

    void drop_leading_non_whitespace()
    {
        while (this->m_current_location < m_size)
        {
            if (isspace(this->m_mem[this->m_current_location]))
            {
                break;
            }
            this->m_current_location++;
        }
    }

    bool parse_token(const char *token, size_t max_size)
    {
        this->drop_leading_whitespace();
        size_t i = 0;
        for (; (i < max_size) && ((this->m_current_location + i) < this->m_size);
             i++)
        {
            if (this->m_mem[this->m_current_location + i] != token[i])
            {
                return false;
            }
        }
        this->m_current_location += i;
        return true;
    }

    bool parse_float(float *out)
    {
        errno = 0;
        char *endptr = NULL;
        *out = strtof(this->m_mem + this->m_current_location, &endptr);
        if ((errno != 0) || (endptr == (this->m_mem + this->m_current_location)))
        {
            return false;
        }
        this->m_current_location +=
            (endptr - (this->m_mem + this->m_current_location));
        return true;
    }

    bool is_end() { return this->m_current_location >= this->m_size; }

    void free() { delete[] this->m_mem; }
};
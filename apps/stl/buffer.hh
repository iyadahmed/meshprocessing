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
        m_mem = new char[size];
        m_size = size;
        m_current_location = 0;
    }

    size_t location() { return m_current_location; }

    size_t size() { return m_size; }

    void printn(size_t n)
    {
        for (int i = 0; (i < n) && ((i + m_current_location) < m_size);
             i++)
        {
            putchar(m_mem[m_current_location + i]);
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
        while (m_current_location < m_size)
        {
            if (!isspace(m_mem[m_current_location]))
            {
                break;
            }
            m_current_location++;
        }
    }

    void drop_leading_non_whitespace()
    {
        while (m_current_location < m_size)
        {
            if (isspace(m_mem[m_current_location]))
            {
                break;
            }
            m_current_location++;
        }
    }

    bool parse_token(const char *token, size_t max_size)
    {
        drop_leading_whitespace();
        size_t i = 0;
        for (; (i < max_size) && ((m_current_location + i) < m_size);
             i++)
        {
            if (m_mem[m_current_location + i] != token[i])
            {
                return false;
            }
        }
        m_current_location += i;
        return true;
    }

    bool parse_float(float *out)
    {
        errno = 0;
        char *endptr = NULL;
        *out = strtof(m_mem + m_current_location, &endptr);
        if ((errno != 0) || (endptr == (m_mem + m_current_location)))
        {
            return false;
        }
        m_current_location +=
            (endptr - (m_mem + m_current_location));
        return true;
    }

    bool is_end() { return m_current_location >= m_size; }

    void free() { delete[] m_mem; }
};
#ifndef CHARARRAY_H
#define CHARARRAY_H

#include <cstddef>
#include <string>
#include <iostream>

class CharArray
{
    public:
        CharArray() = delete;
        CharArray(size_t length);
        virtual ~CharArray();
        CharArray(const CharArray& other);
        CharArray& operator=(const CharArray& other);
        CharArray(CharArray&& other);
        CharArray& operator=( CharArray&& other);
        void Clear( char f = '\0');
        size_t GetSize() const;
        char *GetPtr();
        std::string GetString() const;
    protected:
    private:
        char *pcdata;
        size_t length;
};

inline
std::ostream & operator<< ( std::ostream &os, CharArray const &ca)
{
    os << ca.GetString();
    return os;
}

#endif // CHARARRAY_H

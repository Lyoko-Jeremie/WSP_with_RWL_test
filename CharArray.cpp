#include "CharArray.h"

#include <string>

//CharArray::CharArray():pcdata(nullptr),length(0)
//{
//}

CharArray::CharArray(size_t l):pcdata(nullptr),length(l)
{
    pcdata = new char[length+1];        // 只是多申请和初始化一位  后期的使用还是正常的长度
    for ( size_t i = 0; i != length+1; ++i)
    {
        pcdata[i] = '\0';
    }
}

CharArray::~CharArray()
{
    //dtor
    delete[] pcdata;
    pcdata = nullptr;
    length = 0;
}

CharArray::CharArray(const CharArray& other)
{
    //copy ctor
    this->length = other.length;
    this->pcdata = new char[ this->length + 1];        // 只是多申请和初始化一位  后期的使用还是正常的长度
    for ( size_t i = 0; i != length; ++i)
    {
        this->pcdata[i] = other.pcdata[i];
    }
    this->pcdata[length] = '\0';        // 设置预留位
}

CharArray& CharArray::operator=(const CharArray& rhs)
{
    if (this == &rhs) return *this; // handle self assignment
    //assignment operator

    delete[] this->pcdata;
//    this->pcdata = nullptr;
//    this->length = 0;

    this->length = rhs.length;
    this->pcdata = new char[ this->length + 1];        // 只是多申请和初始化一位  后期的使用还是正常的长度
    for ( size_t i = 0; i != length; ++i)
    {
        this->pcdata[i] = rhs.pcdata[i];
    }
    this->pcdata[length] = '\0';        // 设置预留位

    return *this;
}

CharArray::CharArray(CharArray&& other):pcdata(other.pcdata),length(other.length)
{
    // move ctor
//    this->length = other.length;
//    this->pcdata = other.pcdata;

    other.length = 0;
    other.pcdata = nullptr;

}

CharArray& CharArray::operator=(CharArray&& rhs)
{
    if (this == &rhs) return *this;

    this->length = rhs.length;
    this->pcdata = rhs.pcdata;

    rhs.length = 0;
    rhs.pcdata = nullptr;

    return *this;
}

size_t CharArray::GetSize() const
{
    return this->length;
}

char *CharArray::GetPtr()
{
    return this->pcdata;
}

std::string CharArray::GetString() const
{
    return  std::string(this->pcdata) ;
}

void CharArray::Clear(char f)
{
    for ( size_t i = 0; i != length; ++i)
    {
        this->pcdata[i] = f;
    }
    this->pcdata[length] = '\0';        // 设置预留位
}


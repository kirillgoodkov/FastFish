#pragma once
#include "Common/Common.h"

namespace FastFish{

template <typename TYPE>
class ArrayPtr
{
    ArrayPtr(const ArrayPtr&);
    ArrayPtr& operator = (ArrayPtr);
public:
    ArrayPtr()                                  throw();
    ArrayPtr(size_t nSize)                      throw();
    ~ArrayPtr()                                 throw()     {delete [] m_pData;}
    
    void Clear()                                throw();
    void Alloc(size_t nSize)                    throw();
    void Swap(ArrayPtr& other)                  throw();
    
    TYPE& operator [] (size_t n)                throw();
    TYPE* Get()                                 throw();
    
    const TYPE& operator [] (size_t n)    const throw();
    const TYPE* Get()                     const throw();
    
    bool IsEmpty()                        const throw()     {return 0 == m_pData;}
private:

    TYPE*               m_pData;
    ffDebugOnly(size_t  m_nSize);
};

}
#include "Memory/ArrayPtr.inl"

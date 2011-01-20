#pragma once
#include "Common/Common.h"

namespace FastFish{

class MemMap
{
    MemMap(const MemMap&);
    MemMap& operator = (const MemMap&);
public:
    MemMap(const char* pszName) ffThrowAll;
    ~MemMap() throw();
    
    const void*const    pData;
    const size_t        nSize;
private:    
    #ifdef _MSC_VER
        typedef handle_t file_t;
    #else
        typedef int file_t;
    #endif    
    file_t   m_hFile;             
    handle_t m_hMap;
};

}//namespace FastFish

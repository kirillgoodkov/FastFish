#include "Tools/MemMap.h"

#ifdef _MSC_VER
    extern "C"
    {
        void* __stdcall CreateFileA(const char*, unsigned long, unsigned long, void*, unsigned long, unsigned long, void* );
        void* __stdcall CreateFileMappingA(void*, void*, unsigned long, unsigned long, unsigned long, const char*);
        long __stdcall  CloseHandle(void*);        
        void* __stdcall MapViewOfFile(void*, unsigned long, unsigned long, unsigned long, size_t);
    };

    const unsigned long FILE_READ_DATA          = 0x01;
    const unsigned long FILE_SHARE_READ         = 0x01;
    const unsigned long OPEN_EXISTING           = 3;
    const unsigned long FILE_ATTRIBUTE_NORMAL   = 0x80;
    const unsigned long PAGE_READONLY           = 0x02;
    const unsigned long FILE_MAP_READ           = 0x04;
    const void*         INVALID_HANDLE_VALUE    = (void*)(size_t(-1));    
#else
    #include <sys/mman.h>        
    #include <fcntl.h>
#endif    


using namespace std;

namespace FastFish{

inline
void ThrowError(const char* pszName) ffThrowAll
{
    string strMsg("can't find/open file: ");
    strMsg.append(pszName);
    throw ExceptIO(strMsg);
}

MemMap::MemMap(const char* pszName) ffThrowAll:
    pData(0),
    nSize(0),
    m_hFile(0),
    m_hMap(0)
{
    FILE* pFile = fopen(pszName, "rb");
    if (0 == pFile)
        ThrowError(pszName);
        
    fpos_t nPos = 0;
    if (0 != fseek(pFile, 0, SEEK_END) ||
        0 != fgetpos(pFile, &nPos) ||
        0 == nPos ||
        SIZE_MAX < nPos)
    {
        fclose(pFile);
        ThrowError(pszName);
    }        
    fclose(pFile);    
    const_cast<size_t&>(nSize) = size_t(nPos);
    
    #ifdef _MSC_VER        
        m_hFile = CreateFileA(pszName, FILE_READ_DATA, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (INVALID_HANDLE_VALUE == m_hFile)
            ThrowError(pszName);
            
        m_hMap = CreateFileMappingA(m_hFile, 0, PAGE_READONLY, 0, 0, 0);
        if (0 == m_hMap)
        {
            CloseHandle(m_hFile);
            ThrowError(pszName);
        }        
        const_cast<void*&>(pData) = MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, 0);
        
        if (0 == pData)
        {
            CloseHandle(m_hMap);
            CloseHandle(m_hFile);
            ThrowError(pszName);
        }    
    #else
        m_hFile = open(pszName, O_RDONLY);
        if (-1 == m_hFile)
            ThrowError(pszName);
            
        m_hMap = mmap(0, nSize, PROT_READ, MAP_SHARED, m_hFile, 0);
        if (MAP_FAILED == m_hMap)            
        {
            close(m_hFile);
            ThrowError(pszName);
        }
        const_cast<void*&>(pData) = m_hMap;        
    #endif        
}

MemMap::~MemMap() throw()
{
    #ifdef _MSC_VER        
        CloseHandle(m_hMap);
        CloseHandle(m_hFile);
    #else
        munmap(m_hMap, nSize);
        close(m_hFile);
    #endif        
}


}
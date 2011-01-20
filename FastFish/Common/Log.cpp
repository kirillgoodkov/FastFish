#include "Tools/Bits.h"
#include "Common/Common.h"
#include <cassert>

namespace FastFish{

FILE* g_arrLogs[ffBitSizeOf(int)] = 
{
    stderr, stderr, stderr, stderr, stderr, stderr, stderr, stderr, //lsBug
    stderr, stderr, stderr, stderr, stderr, stderr, stderr, stderr, //lsExcept
    stderr, stderr, stderr, stderr, stderr, stderr, stderr, stderr, //lsWarn
    stderr, stderr, stderr, stderr, stderr, stderr, stderr, stderr, //lsInfo
};

void __cdecl SetLog(FILE* pLog, LogStream ls) throw()
{
    for (size_t n = 0; n < ffCountOf(g_arrLogs); ++n)
    {
        if (ls | (1 << n))
        {
            g_arrLogs[n] = pLog;
        }
    }
} 

bool _Log(LogStream ls, const char* pszFormat, ...) throw()
{
    if (0 == (ls & lsAll))
        return false;
        
    FILE* pLog = g_arrLogs[CountRightNulls(uns4_t(ls))];
    
    if (pLog)
    {
        va_list pArgs;
        va_start(pArgs, pszFormat);      
        vfprintf(pLog, pszFormat, pArgs);       
        ffFFlush(pLog);
        va_end(pArgs);    
    }        
    
    if (lsBug & ls)             
        assert(false);
    
    return true;
}

}//namespace FastFish

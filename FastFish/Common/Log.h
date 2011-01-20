#pragma once

#define ffIsLogged(_LS)     (Static(0 != (_LS & lsAll)) && g_arrLogs[CountRightNulls(uns4_t(_LS))])
#define ffLog(_LS, ...)     ((void)((_LS & lsAll) &&  _Log(_LS, __VA_ARGS__)))
#define ffLogNN(_LS, ...)   ((void)((_LS & lsAll) &&  _Log(_LS, "\n") && _Log(_LS, __VA_ARGS__) && _Log(_LS, "\n")))

namespace FastFish{

inline
bool Static(bool b) throw() //for prevent warning "conditional expression is constant"
{
    return b;
}

enum LogStream
{
    lsBugInternal           = 0x00000001,
    lsBugUser               = 0x00000002,
    lsBug                   = lsBugInternal | lsBugUser,
    
    lsExceptIO              = 0x00000100,
    lsExceptSyntax          = 0x00000200,
    lsExceptFileFormat      = 0x00000400,
    lsExcept                = lsExceptIO | lsExceptSyntax | lsExceptFileFormat,
    
    lsWarnWordCut           = 0x00010000,
    lsWarnStringCut         = 0x00020000,
    lsWarnInvalidUTF8       = 0x00040000,
    lsWarn                  = lsWarnStringCut /*| lsWarnWordCut | lsWarnInvalidUTF8*/,

    lsInfoFileSummary       = 0x01000000,
    lsInfoTopPrefix         = 0x02000000,
    lsInfoQueryOriginal     = 0x04000000,
    lsInfoQueryOptimized    = 0x08000000,    
    lsInfo                  = lsInfoFileSummary /*| lsInfoTopPrefix | lsInfoQueryOriginal | lsInfoQueryOptimized*/,
    
    lsAll                   = lsBug | lsExcept | lsWarn | lsInfo    
};

void __cdecl SetLog(FILE* pLog, LogStream ls = lsAll) throw();
bool _Log(LogStream ls, const char* pszFormat, ...) throw();

extern FILE* g_arrLogs[];

}//namespace FastFish


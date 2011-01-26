#pragma once

#ifdef _DEBUG
    #define ffDebug
#else   
    #define ffRelease
#endif

#ifdef __GNUG__
    #define ffThrowAll
    #define ffForceInline   inline
    #define ffTouch(_Var)   Static(0 == _Var)
    #ifdef __USE_MISC
        #define ffFWrite    fwrite_unlocked
        #define ffFFlush    fflush_unlocked
    #else
        #define ffFWrite    fwrite
        #define ffFFlush    fflush
    #endif                
    
    #if ((4 == __GNUC__ && 5 <= __GNUC_MINOR__) || 4 < __GNUC__)
        #define ffReturnNop         __builtin_unreachable()
        #define __assume(_Expr)     for(;;){if (!(_Expr)) __builtin_unreachable();break;}
    #else        
        #define ffReturnNop         return 0
        #define __assume(_Expr) 
    #endif        
    
    #if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
        #define ffLittleEndian
    #endif

    #ifndef _X86_
        #define ffStrictAlign
    #endif        
    
    #ifdef __LP64__
        #define ff64
    #else
        #define ff32
    #endif     
#elif _MSC_VER 
    #define ffThrowAll      throw(...)        
    #define ffForceInline   __forceinline
    #define ffTouch(_Var)   _Var
    #define ffFWrite        _fwrite_nolock
    #define ffFFlush        _fflush_nolock
    
    #if !defined(_M_ARM) && !defined(_M_PPC)
        #define ffLittleEndian
    #endif        
    
    #ifndef _M_IX86
        #define ffStrictAlign
    #endif
    
    #ifdef _WIN64
        #define ff64
    #else
        #define ff32
    #endif
    
    #ifdef __INTEL_COMPILER
        #define ffReturnNop for(;;){__assume(false); return 0;}
    #else
        #define ffReturnNop __assume(false)
    #endif            

    #ifdef ffRelease
        #define _SECURE_SCL 0
        #define _SECURE_SCL_THROWS 0
    #else
        #define _SCL_SECURE_NO_WARNINGS            
    #endif            

    extern "C" {void __stdcall DebugBreak();}        
#else
    #error Unknown compiler 
#endif    
    

#ifdef ff64
    #define ff64Only(_Expr) _Expr
    #define ff32Only(_Expr) 
    #define ff32Or64(_Expr32, _Expr64) _Expr64
#else
    #define ff64Only(_Expr) 
    #define ff32Only(_Expr) _Expr
    #define ff32Or64(_Expr32, _Expr64) _Expr32
#endif

#ifdef ffDebug
    #define ff_LogInternal(_Expr)       ((void)(!!(_Expr) || _Log(lsBugInternal, "\ninternal bug; %s; %u\n", __FILE__, __LINE__)))             
    #define ff_LogUser(_Expr, _Text)    ((void)(!!(_Expr) || _Log(lsBugUser, "\nuser bug; %s; %u; %s\n", __FILE__, __LINE__, _Text)))             

    #define ffAssert(_Expr)             ff_LogInternal(_Expr)
    #define ffAssertUser(_Expr, _Text)  ff_LogUser(_Expr, _Text)
    #define ffAssume(_Expr)             for(;;){ff_LogInternal(_Expr);__assume(_Expr);break;}
    #define ffAssumeStatic(_Expr)       for(;;){__assume(_Expr); ff_LogInternal(_Expr);break;}//produce level4 warning C4702(only in debug config, MSVC)
    #define ffAssumeUser(_Expr, _Text)  for(;;){ff_LogUser(_Expr, _Text); __assume(_Expr);break;}
    #define ffVerify(_Expr)             ff_LogInternal(_Expr)
    #define ffDebugOnly(_Expr)          _Expr
    #define ffDbgOrRel(_ExprD, _ExprR)  _ExprD
    
    //#define ffDebugDeepChecks
#else   
    #define ffAssert(_Expr)             ((void)0)
    #define ffAssertUser(_Expr, _Text)  ((void)0)
    #define ffAssume(_Expr)             __assume(_Expr)
    #define ffAssumeStatic(_Expr)       __assume(_Expr)
    #define ffAssumeUser(_Expr, _Text)  __assume(_Expr)
    #define ffVerify(_Expr)             ((void)(!!(_Expr)))
    #define ffDebugOnly(_Expr)                  
    #define ffDbgOrRel(_ExprD, _ExprR)  _ExprR
#endif    

#define ffCountOf(_Array)           (sizeof(_Array) / sizeof(_Array[0]))
#define ffBitSizeOf(_TYPE)          (sizeof(_TYPE)*BitsInByte)
#define ffAllocAuto(_TYPE, _COUNT)  (static_cast<_TYPE*>(alloca(sizeof(_TYPE)*(_COUNT))))

//for special purpose: method throws bad_alloc and has local objects with allocated resources
#define ffThrowNone

#include <cstddef>
#include <cstdio>
#include <cstdarg>
#include <stdexcept>
#include <cstdio>
#include <memory.h>
#include <vector>
#include <list>
#include <string>
#include <memory>
#include <algorithm>
#include <functional>

#include "Common/Log.h"
#include "Common/Types.h"


#pragma once
#include "Common/Common.h"

namespace FastFish{

template<class OUTTYPE, class INTYPE> inline
OUTTYPE down_cast(INTYPE var) throw()
{
    ffAssert(dynamic_cast<OUTTYPE>(var));
    return static_cast<OUTTYPE>(var);
}

template<typename TYPE, typename PRED = std::less<TYPE> >
struct InderectCmp
{
    const TYPE* arr;
    PRED        pred;

    InderectCmp(const TYPE a[]) throw()                 : arr(a) {}            
    bool operator ()(size_t a, size_t b) const throw()  {return pred(arr[a], arr[b]);}    
};

template<typename TYPE, typename PRED>
bool PtrCmp(const TYPE* pA, const TYPE* pB) throw()
{
    return PRED()(*pA, *pB);
}

template<typename TYPE>
TYPE* AllocPODVar(byte_t*& pMem, size_t n) throw()
{
    TYPE* p = reinterpret_cast<TYPE*>(pMem);
    pMem += TYPE::SizeOf(n);    
    return p;
}

template<typename TYPE>
TYPE* AllocPODFix(byte_t*& pMem) throw()
{
    TYPE* p = reinterpret_cast<TYPE*>(pMem);
    pMem += sizeof(TYPE);    
    return p;
}

inline
void CheckVersion(uns1_t nVerActual, uns1_t nVerSupported) throw()
{
    ffAssume(nVerActual <= nVerSupported);
}

inline
bool CompareRS(Document docA, Document docB) throw() 
{//equal to: rank < oth.rank || (rank == oth.rank && sid <= oth.sid)
    return 
        #ifdef ffLittleEndian    
            *(const uns4_t*)((const byte_t*)&docA + sizeof(uns4_t)) <= 
            *(const uns4_t*)((const byte_t*)&docB + sizeof(uns4_t)) ;
        #else
            *(const uns4_t*)(&docA) <= 
            *(const uns4_t*)(&docB) ;
        #endif           
}

}//namespace FastFish

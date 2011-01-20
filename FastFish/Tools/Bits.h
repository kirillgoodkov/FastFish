#pragma once
#include "Common/Common.h"

namespace FastFish {

template<typename TYPE>
TYPE MaxLessOrEqualPow2(TYPE n) throw();

template<typename TYPE>
TYPE MinGreateOrEqualPow2(TYPE n) throw();

template<typename TYPE>
size_t CountRightNulls(TYPE n) throw();

template<typename TYPE>
size_t CountLeftNulls(TYPE n) throw();

extern const uns1_t g_arrTableCB[SizeUns1];

template<typename TYPE>
uns1_t CountBits(TYPE n) throw();

template<typename TYPE>
size_t Value2SizeOf(TYPE nVal) throw();

template<typename TYPE>
TYPE EqualCount(const void* pA, const void* pB, TYPE nLen) throw();

template<typename TYPE>
TYPE ReadVal(const byte_t* p, size_t nSizeOf) throw();

template<typename TYPE>
void SetBit(TYPE* arrBmp, size_t nBit) throw();
template<typename TYPE>
bool GetBit(const TYPE* arrBmp, size_t nBit) throw();
template<typename TYPE>
size_t FindBit(const TYPE* arrBmp, size_t nFrom) throw();

//variable byte encoding
uns4_t VbeRead(const uns1_t*& p) throw();
void   VbeWrite(uns4_t nVal, uns1_t*& p) throw();
size_t VbeSizeOf(uns4_t nVal) throw();

}//namespace FastFish 

#include "Tools/Bits.inl"

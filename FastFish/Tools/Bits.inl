#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
    #include <intrin.h>
#endif    

namespace FastFish {

template<> inline 
uns1_t MaxLessOrEqualPow2(uns1_t n) throw()
{
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    return n - (n >> 1);
}

template<> inline 
uns2_t MaxLessOrEqualPow2(uns2_t n) throw()
{
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    return n - (n >> 1);
}

template<> inline 
uns4_t MaxLessOrEqualPow2(uns4_t n) throw()
{
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n - (n >> 1);
}

template<> inline 
uns8_t MaxLessOrEqualPow2(uns8_t n) throw()
{
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return n - (n >> 1);
}

template<> inline 
uns1_t MinGreateOrEqualPow2(uns1_t n) throw()
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    return ++n;
}

template<> inline 
uns2_t MinGreateOrEqualPow2(uns2_t n) throw()
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    return ++n;
}

template<> inline 
uns4_t MinGreateOrEqualPow2(uns4_t n) throw()
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return ++n;
}

template<> inline 
uns8_t MinGreateOrEqualPow2(uns8_t n) throw()
{
    --n;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    return ++n;
}

template<> inline
size_t CountRightNulls(uns4_t x) throw()
{
    if (0 == x)
        return ffBitSizeOf(x);
        
    #ifdef __GNUG__        
        return __builtin_ctzl(x);
    #elif _MSC_VER
        unsigned long n;
        _BitScanForward(&n, x);
        return n;
    #else
        size_t n = 1;
        if (0 == (x & 0x0000FFFF)) { n += 16; x >>= 16;}
        if (0 == (x & 0x000000FF)) { n +=  8; x >>=  8;}
        if (0 == (x & 0x0000000F)) { n +=  4; x >>=  4;}
        if (0 == (x & 0x00000003)) { n +=  2; x >>=  2;}
        return n - (x & 1);
    #endif    
}

template<> inline
size_t CountRightNulls(uns8_t x) throw()
{
    if (0 == x)
        return ffBitSizeOf(x);

    #ifdef __GNUG__        
        return __builtin_ctzll(x);
    #elif (defined(_MSC_VER) && defined(ff64))
        unsigned long n;
        _BitScanForward64(&n, x);
        return n;
    #else
        size_t n = 1;
        if (0 == (x & 0x00000000FFFFFFFFull)) { n += 32; x >>= 32;}
        if (0 == (x & 0x000000000000FFFFull)) { n += 16; x >>= 16;}
        if (0 == (x & 0x00000000000000FFull)) { n +=  8; x >>=  8;}
        if (0 == (x & 0x000000000000000Full)) { n +=  4; x >>=  4;}
        if (0 == (x & 0x0000000000000003ull)) { n +=  2; x >>=  2;}
        return n - (size_t(x) & 1);
    #endif    
}

extern const uns1_t g_arrTableCLN[SizeUns1];

template<> inline
size_t CountLeftNulls(uns4_t x) throw()
{
    if (0 == x)
        return ffBitSizeOf(x);
        
    #ifdef __GNUG__        
        return __builtin_clzl(x);
    #elif _MSC_VER
        unsigned long n;
        _BitScanReverse(&n, x);
        return ffBitSizeOf(x) - 1 - n;
    #else
        size_t n = 32;
        uns4_t y = x >> 16; if (0 != y) {n  = 16; x = y;}
               y = x >>  8; if (0 != y) {n -=  8; x = y;}
        return n - g_arrTableCLN[x];
    #endif
}

template<> inline
size_t CountLeftNulls(uns8_t x) throw()
{
    if (0 == x)
        return ffBitSizeOf(x);
        
    #ifdef __GNUG__        
        return __builtin_clzll(x);
    #elif (defined(_MSC_VER) && defined(ff64))
        unsigned long n;
        _BitScanReverse64(&n, x);
        return ffBitSizeOf(x) - 1 - n;
    #else
        size_t n = 64;
        uns8_t y = x >> 32; if (0 != y) {n  = 32; x = y;}
               y = x >> 16; if (0 != y) {n -= 16; x = y;}
               y = x >>  8; if (0 != y) {n -=  8; x = y;}
        return n - g_arrTableCLN[x];
    #endif    
}

template<> inline
uns1_t CountBits(uns8_t n) throw()
{
    return g_arrTableCB[uns1_t(n      )] +
           g_arrTableCB[uns1_t(n >>  8)] +
           g_arrTableCB[uns1_t(n >> 16)] +
           g_arrTableCB[uns1_t(n >> 24)] +
           g_arrTableCB[uns1_t(n >> 32)] +
           g_arrTableCB[uns1_t(n >> 40)] +
           g_arrTableCB[uns1_t(n >> 48)] +
           g_arrTableCB[uns1_t(n >> 56)] ;
}

template<> inline
uns1_t CountBits(uns4_t n) throw()
{
    return g_arrTableCB[uns1_t(n      )] +
           g_arrTableCB[uns1_t(n >>  8)] +
           g_arrTableCB[uns1_t(n >> 16)] +
           g_arrTableCB[uns1_t(n >> 24)] ;
}

template<typename TYPE> 
size_t Value2SizeOf(TYPE nVal) throw()
{
    size_t nRet = (ffBitSizeOf(uns8_t) - CountLeftNulls(uns8_t(nVal) - 1) + BitsInByte - 1)/BitsInByte;
    return 0 < nRet ? nRet : 1;
}

template<typename TYPE> 
TYPE EqualCount(const void* _pA, const void* _pB, TYPE nLen) throw()
{
    const byte_t* pA = reinterpret_cast<const byte_t*>(_pA);
    const byte_t* pB = reinterpret_cast<const byte_t*>(_pB);
    TYPE n = 0;
    for (; n < nLen && *pA++ == *pB++; ++n)
    {
    }
    return n;
}

inline
uns8_t _ReadValMask(uns8_t n, size_t nSizeOf) throw()
{
    static const uns8_t arrMasks[] = 
    {
        0x00000000000000FFull, 
        0x000000000000FFFFull, 
        0x0000000000FFFFFFull, 
        0x00000000FFFFFFFFull, 
        0x000000FFFFFFFFFFull, 
        0x0000FFFFFFFFFFFFull, 
        0x00FFFFFFFFFFFFFFull,
        0xFFFFFFFFFFFFFFFFull
    };
    return n & arrMasks[nSizeOf - 1];
}

inline
uns4_t _ReadValMask(uns4_t n, size_t nSizeOf) throw()
{
    static const uns4_t arrMasks[] = 
    {
        0x000000FF, 
        0x0000FFFF, 
        0x00FFFFFF, 
        0xFFFFFFFF
    };
    return n & arrMasks[nSizeOf - 1];
}

inline
uns2_t _ReadValMask(uns2_t n, size_t nSizeOf) throw()
{
    static const uns2_t arrMasks[] = 
    {
        0x00FF, 
        0xFFFF
    };        
    return n & arrMasks[nSizeOf - 1];
}

template<typename TYPE> 
TYPE ReadVal(const byte_t* p, size_t nSizeOf) throw()
{
    ffAssume(nSizeOf && nSizeOf <= sizeof(TYPE));
    #ifndef ffLittleEndian
        p -= sizeof(TYPE) - nSizeOf;
    #endif    
    
    TYPE n;     
    #ifdef ffStrictAlign
        memcpy(&n, p, sizeof(TYPE));
    #else 
        n = *reinterpret_cast<const TYPE*>(p);
                
    #endif                
    return _ReadValMask(n, nSizeOf);
}

template<> inline
void SetBit(uns1_t* arrBmp, size_t nBit) throw()
{
    arrBmp[nBit >> 3] |= 1 << (nBit & 7);
}
template<> inline
void SetBit(uns2_t* arrBmp, size_t nBit) throw()
{
    arrBmp[nBit >> 4] |= 1 << (nBit & 15);
}
template<> inline
void SetBit(uns4_t* arrBmp, size_t nBit) throw()
{
    arrBmp[nBit >> 5] |= 1 << (nBit & 31);
}
template<> inline
void SetBit(uns8_t* arrBmp, size_t nBit) throw()
{
    arrBmp[nBit >> 6] |= 1ull << (nBit & 63ull);
}

template<> inline
bool GetBit(const uns1_t* arrBmp, size_t nBit) throw()
{
    return 0 != (arrBmp[nBit >> 3] & (1 << (nBit & 7)));
}
template<> inline
bool GetBit(const uns2_t* arrBmp, size_t nBit) throw()
{
    return 0 != (arrBmp[nBit >> 4] & (1 << (nBit & 15)));
}
template<> inline
bool GetBit(const uns4_t* arrBmp, size_t nBit) throw()
{
    return 0 != (arrBmp[nBit >> 5] & (1 << (nBit & 31)));
}
template<> inline
bool GetBit(const uns8_t* arrBmp, size_t nBit) throw()
{
    return 0 != (arrBmp[nBit >> 6] & (1ull << (nBit & 63ull)));
}

template<typename TYPE> 
size_t FindBit(const TYPE* p, size_t n) throw()
{
    const size_t cBSO = ffBitSizeOf(TYPE);
    p += n/cBSO;    
    
    size_t nBeg = n % cBSO;

    size_t nAdd = CountRightNulls(*p >> nBeg); 
    if (cBSO == nAdd)
    {
        const TYPE* pCur = p;
        do
        {
            nAdd = CountRightNulls(*++pCur);
        }
        while (cBSO == nAdd);
        
        return n - nBeg + nAdd + cBSO*(pCur - p);             
    }
    else
    {
        return n + nAdd;
    }
}

inline
size_t VbeSizeOf(uns4_t nVal) throw()
{
    static const size_t arrSizes[] = 
    {
        1, 1, 1, 1, 1, 1, 1, 1,
           2, 2, 2, 2, 2, 2, 2,
           3, 3, 3, 3, 3, 3, 3,
           4, 4, 4, 4, 4, 4, 4,
           5, 5, 5, 5,
    };
    ffAssume(ffCountOf(arrSizes) == ffBitSizeOf(nVal) + 1);
    
    size_t nBits = ffBitSizeOf(nVal) - CountLeftNulls(nVal);
    ffAssume(nBits <= ffBitSizeOf(nVal));           
    return arrSizes[nBits];
}


inline
uns4_t VbeRead(const uns1_t*& p) throw()
{
    ffAssume(0x80 != *p);
    
    //big endian store
    if (*p & 0x80)
    {
        uns4_t n = *p & 0x7F;
        while (*++p & 0x80)
        {
            n = (n << 7) | (*p & 0x7F);
        }
        return (n << 7) | *p++;
    }
    else
    {
        return uns4_t(*p++);
    }

}

inline
void VbeWrite(uns1_t*& p, uns4_t nVal) throw()
{
    size_t nBits = ffBitSizeOf(nVal) - CountLeftNulls(nVal);
    ffAssume(nBits <= ffBitSizeOf(nVal));

    static const uns1_t arrShifts[] = 
    {
        0,  0,  0,  0,  0,  0,  0,  0, 
            7,  7,  7,  7,  7,  7,  7, 
           14, 14, 14, 14, 14, 14, 14, 
           21, 21, 21, 21, 21, 21, 21, 
           28, 28, 28, 28, 
    };
    ffAssumeStatic(ffCountOf(arrShifts) == ffBitSizeOf(nVal) + 1);

    //big-endian store
    for (uns1_t nShift = arrShifts[nBits]; nShift; nShift -= 7)
    {
        *p++ = uns1_t(nVal >> nShift) | 0x80;
    }
    *p++ = uns1_t(nVal) & 0x7F;    
}

}//namespace FastFish 

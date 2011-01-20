#include "Tools/MemManager.h"
#include "Common/Consts.h"

#ifdef _MSC_VER
    extern "C"
    {
        long __stdcall SetProcessWorkingSetSize(void*, size_t, size_t);
        long __stdcall GetProcessWorkingSetSize(void*, size_t*, size_t*);
        long __stdcall VirtualLock(const void* lpAddress, size_t dwSize);
    }
    void* CURRENT_PROC = (void*)(-1);
    
    #define ffMLock VirtualLock
#else
    #include <sys/mman.h>        
    
    #define ffMLock mlock
#endif    

namespace FastFish{

inline
void MemManager::RaisePages(Bag bg) const throw()
{
    size_t nDumb = 0;
    
    const byte_t* p    = static_cast<const byte_t*>(bg.first); 
    const byte_t* pEnd = static_cast<const byte_t*>(bg.second); 
    
    for (; p < pEnd; p += MinPageSize)
    {
        nDumb += *p;
    }
    nDumb += *(pEnd - 1);            
    m_nDumb = nDumb;
}

void MemManager::AddSegment() throw()
{
    for (VectorLayers::iterator it = m_vecLayers.begin(); it != m_vecLayers.end(); ++it)
    {
        it->push_back(Segment());
    }
}

void MemManager::AddBag(size_t nLayer, const void* p, size_t n) throw()
{
    Ptr pBegin = static_cast<Ptr>(p);
    Ptr pEnd   = pBegin + n;

    Segment& seg = m_vecLayers[nLayer].back();

    if (!seg.empty() && seg.rbegin()->second == pBegin)
    {
        seg.rbegin()->second = pEnd;
    }
    else
    {
        seg.push_back(std::make_pair(pBegin, pEnd));
    }
}

void MemManager::Prefetch(size_t nMaxSize) const ffThrowNone
{
    ffAssertUser(!m_vecLayers[0].empty(), "nothing prefetch");
    
    VectorBags vecBags;
    Iterate(m_vecLayers.size(), nMaxSize, vecBags);

    for (VectorBags::const_reverse_iterator it = vecBags.rbegin(); it != vecBags.rend(); ++it)    
    {
        RaisePages(*it);
    }
}

#define ffCheckAndRet(_Expr) if (!_Expr) return false

bool MemManager::Lock(size_t nLayersCount, size_t nMaxSize) const ffThrowNone
{
    VectorBags vecBags;
    size_t nSize = Iterate(nLayersCount, nMaxSize, vecBags);

    #ifdef _MSC_VER
        size_t nMin, nMax;
        ffCheckAndRet(GetProcessWorkingSetSize(CURRENT_PROC, &nMin, &nMax));
        nSize += m_vecLayers[0].size()*(1u << 20);//+1 MB per each segment, fucked magic
        nMin += nSize;
        nMax += nSize;
        ffCheckAndRet(SetProcessWorkingSetSize(CURRENT_PROC, nMin, nMax));
    #else
        ffTouch(nSize);
    #endif

    for (VectorBags::const_iterator it = vecBags.begin(); it != vecBags.end(); ++it)    
    {
        ffCheckAndRet(ffMLock(it->first, it->second - it->first));
    }    
    return true;
}

size_t MemManager::Iterate(size_t nLayersCount, size_t nMaxSize, VectorBags& vec) const throw()
{
    size_t nSize = 0;    
    for (VectorLayers::const_iterator itLr = m_vecLayers.begin(); itLr != m_vecLayers.begin() + nLayersCount; ++itLr)
    {
        for (Layer::const_reverse_iterator itSeg = itLr->rbegin(); itSeg != itLr->rend(); ++itSeg)
        {
            for (Segment::const_iterator itBag = itSeg->begin(); itBag != itSeg->end(); ++itBag)
            {
                nSize += itBag->second - itBag->first;
                if (nMaxSize < nSize)
                {
                    vec.push_back(std::make_pair(itBag->first, itBag->second - nSize + nMaxSize));
                    return nMaxSize;
                }
                else
                {
                    vec.push_back(*itBag);
                }                
            }
        }
    }
    return nSize;
}

}//namespace FastFish

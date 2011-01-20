#pragma once
#include "Common/Common.h"

namespace FastFish{

class MemManager
{   
    MemManager(const MemManager&);
    MemManager& operator = (const MemManager&);
public:
    MemManager(size_t nLayers) : m_vecLayers(nLayers) {};

    void AddSegment()                                     throw();
    void AddBag(size_t nLayer, const void* p, size_t n)   throw();
    
    void Prefetch(size_t nMaxSize)                  const ffThrowNone;
    bool Lock(size_t nLayersCount, size_t nMaxSize) const ffThrowNone;
    
private:    

    typedef const byte_t*           Ptr;
    typedef std::pair<Ptr, Ptr>     Bag;
    typedef std::vector<Bag>        VectorBags;
    typedef VectorBags              Segment;
    typedef std::vector<Segment>    Layer;
    typedef std::vector<Layer>      VectorLayers;
    
    size_t Iterate(size_t nLayersCount, size_t nMaxSize, VectorBags& vec) const throw();
    void   RaisePages(Bag bg) const throw();
    
    VectorLayers    m_vecLayers;
    mutable size_t  m_nDumb;
};

}//namespace FastFish


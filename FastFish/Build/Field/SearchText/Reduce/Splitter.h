#pragma once
#include "Build/Field/SearchText/Reduce/brNode.h"
#include "Tools/Strings.h"           
#include "Common/Common.h"

namespace FastFish{
namespace Build{
namespace Reduce{

template<typename BASE, bool bCTRL, bool bSENS, TextIndexType tit>
class Splitter:
    public BASE
{
    Splitter(const Splitter&);
    Splitter& operator = (const Splitter&);
public:
    Splitter(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw();
    virtual ~Splitter()                                                                                         throw() {};

    virtual void Add(const void* pData, datasize_t nSize) throw();
        
private:
    typedef Node<bCTRL, bSENS>                    NodeType;
    typedef CharMapFastPd<NodeType, bCTRL, bSENS> MapTypePd;
   
    const StrCSpn       c_cspn;
    const size_t        c_nMaxLen;  
    const MapTypePd     c_mapSkip;
};

}//namespace Reduce
}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/Reduce/Splitter.inl"

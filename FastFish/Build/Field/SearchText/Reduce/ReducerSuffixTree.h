#pragma once
#include "Build/Field/SearchText/Reduce/brNode.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{
namespace Reduce{

template<typename BASE, bool bCTRL, bool bSENS, TextIndexType tit>
class ReducerSuffixTree:
    public BASE
{
    ReducerSuffixTree(const ReducerSuffixTree&);
    ReducerSuffixTree& operator = (const ReducerSuffixTree&);
public:
    ReducerSuffixTree(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw();
    virtual ~ReducerSuffixTree()                                                                                         throw() {};

    virtual void Commit(rank_t rank)                    throw();
    
    void AddWord(const char* pWord, wordsize_t nLen)    throw();
        
private:
    typedef Node<bCTRL, bSENS>                      NodeType;
    typedef CharMapFastPd<NodeType, bCTRL, bSENS>   MapTypePd;
   
    MapTypePd           m_mapRoot;
    
    std::vector<char>   m_vecStrTemp;
};

}//namespace Reduce
}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/Reduce/ReducerSuffixTree.inl"

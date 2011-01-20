#pragma once
#include "Build/Field/SearchText/Reduce/brNode.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{
namespace Reduce{

template<typename BASE, bool bCTRL, bool bSENS>
class ReducerNop:
    public BASE
{
    ReducerNop(const ReducerNop&);
    ReducerNop& operator = (const ReducerNop&);
public:
    ReducerNop(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw();
    virtual ~ReducerNop()                                                                                         throw() {};

    virtual void Commit(rank_t rank)                    throw();
    
    void AddWord(const char* pWord, wordsize_t nLen)    throw();
        
private:
    struct Word
    {
        const char* pStr;
        wordsize_t  nLen;
    };

    std::vector<Word> m_vecWords;    
};

}//namespace Reduce
}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/Reduce/ReducerNop.inl"

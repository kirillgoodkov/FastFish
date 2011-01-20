#pragma once
#include "Containers/CharMapUtils.h"
#include "Memory/AllocatorInvaderBB.h"
#include "Memory/AllocatorInvader.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

struct Field
{                              
    virtual ~Field()                                        throw()     {};
    
    virtual void Add(const void* pData, datasize_t nSize)   throw()     = 0;
    virtual void Commit(rank_t rank)                        throw()     = 0;
    
    virtual void Finalize()                                 ffThrowNone = 0;
    virtual void Store(OFStream& file)                const ffThrowAll  = 0;
};

struct TextAllocator
{
    CharMapAllocator        aMap;
    AllocatorInvader        aSet;
    AllocatorInvaderBB      aStr;

    AllocatorInvader        aRMap;
    AllocatorInvaderBB      aRStr;    
    
    AllocatorInvader        aSkipMap;
    AllocatorInvaderBB      aSkipStr;    

    TextAllocator() throw();
};

Field* CreateFieldDataFix(const TplIndex& tplIndex, const TplFieldDataFix& tpl, const Documents* pDocs) throw();
Field* CreateFieldDataVar(const TplIndex& tplIndex, const TplFieldDataVar& tpl, const Documents* pDocs) throw();
Field* CreateFieldSearchText(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw();

}//namespace Build
}//namespace FastFish


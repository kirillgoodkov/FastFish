#pragma once
#include "Build/Field/bField.h"
#include "Containers/DequeRaw.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

class FieldDataFix:
    public Field
{
    FieldDataFix(const FieldDataFix&);
    FieldDataFix& operator = (const FieldDataFix&);
public:
    FieldDataFix(const TplIndex& tplIndex, const TplFieldDataFix& tpl, const Documents* pDocs) throw();

    virtual void Add(const void* pData, datasize_t nSize) throw();
    virtual void Commit(rank_t rank)                      throw();
        
    virtual void Finalize()                               ffThrowNone   {}
    virtual void Store(OFStream& file)              const ffThrowAll;

protected:        
    template<typename DOCUMENTS>
    void StoreT(OFStream& file, const DOCUMENTS* pDocs) const ffThrowAll;

    const TplIndex&         c_tplIndex;
    const Documents*const   c_pDocs;
    
    DequeRaw                m_deqData;
    ffDebugOnly(bool        m_bAdded);
};

}//namespace Build
}//namespace FastFish


#pragma once
#include "Build/Field/bField.h"
#include "Containers/DequeRaw.h"
#include "Containers/Deque.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

class FieldDataFixOpt:
    public Field
{
    FieldDataFixOpt(const FieldDataFixOpt&);
    FieldDataFixOpt& operator = (const FieldDataFixOpt&);
public:
    FieldDataFixOpt(const TplIndex& tplIndex, const TplFieldDataFix& tpl, const Documents* pDocs) throw();

    virtual void Add(const void* pData, datasize_t nSize) throw();
    virtual void Commit(rank_t rank)                      throw();
        
    virtual void Finalize()                               ffThrowNone   {}
    virtual void Store(OFStream& file)              const ffThrowAll;

protected:        
    template<typename DOCUMENTS>
    void StoreData(OFStream& file, const DOCUMENTS* pDocs, docid_t* arrIdxNew) const ffThrowAll;
    
    typedef Deque<docid_t, PC::FieldDataFixDeqShift> DequeIdx;

    const TplIndex&         c_tplIndex;
    const Documents*const   c_pDocs;
    
    DequeRaw                m_deqData;
    DequeIdx                m_deqIdx;
    bool                    m_bAdded;
    
};

}//namespace Build
}//namespace FastFish


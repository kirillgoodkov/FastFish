#pragma once
#include "Build/Field/bField.h"
#include "Containers/Deque.h"
#include "Memory/AllocatorInvaderBB.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

template<bool bZT, bool bSZ>
class FieldDataVar:
    public Field
{
    FieldDataVar(const FieldDataVar&);
    FieldDataVar& operator = (const FieldDataVar&);
public:
    FieldDataVar(const TplIndex& tplIndex, const TplFieldDataVar& tpl, const Documents* pDocs) throw();
        
    virtual void Add(const void* pData, datasize_t nSize) throw();
    virtual void Commit(rank_t rank)                      throw();
        
    virtual void Finalize()                               ffThrowNone   {}
    virtual void Store(OFStream& file)              const ffThrowAll;

private:
    template<typename POS_TYPE, typename SIZE_TYPE, typename DOCUMENTS>
    void StoreT(OFStream& file, const DOCUMENTS* pDocs) const ffThrowAll;    
    
    typedef Deque<datasize_t, PC::FieldDataVarDeqSizesShift>  DeqSizes;
    typedef Deque<void*, PC::FieldDataVarDeqPointersShift>    DeqPointers;
    
    const TplIndex&         c_tplIndex;
    const TplFieldDataVar&  c_tpl;
    const Documents*const   c_pDocs;

    AllocatorInvaderBB      m_aUserData;
    
    DeqSizes                m_deqSizes;
    DeqPointers             m_deqPointers;
    size_t                  m_nMaxPos;
    datasize_t              m_nMaxSize;    
    ffDebugOnly(bool        m_bAdded);


};


}//namespace Build
}//namespace FastFish



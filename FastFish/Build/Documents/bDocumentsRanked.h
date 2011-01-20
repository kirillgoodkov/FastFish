#pragma once
#include "Build/Documents/bDocuments.h"
#include "Containers/DequeA.h"
#include "Memory/ArrayPtr.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

class DocumentsRanked:
    public Documents
{
    DocumentsRanked(const DocumentsRanked&);
    DocumentsRanked& operator = (const DocumentsRanked&);
public:
    DocumentsRanked(const TplIndex& tpl)        throw();

    virtual docid_t Add(rank_t rank)              throw();    
    virtual docid_t Count()                 const throw()       {return m_didCount;}
    
    virtual void    Finalize()                    throw();
    virtual void    Store(OFStream& file)   const ffThrowAll;
    
    docid_t TransformForw(docid_t did)      const throw();
    docid_t TransformBack(docid_t did)      const throw();
    
private:      

    typedef DequeA<docid_t, PC::DocumentsDeqShift> DequeDocuments;
    typedef ArrayPtr<DequeDocuments>               ArrayDequeDocuments;
    typedef ArrayPtr<docid_t>                      ArrayDids;
    
    const TplIndex&     c_tpl;
    
    AllocatorInvader    m_a;
    ArrayDequeDocuments m_arrLists;    
    docid_t             m_didCount;
    
    //valid after finalize
    ArrayDids   m_arrIndexForw;
    ArrayDids   m_arrIndexBack;
    ArrayDids   m_arrIndexRanks;
    rank_t      m_nRanks;
    
};

}//namespace Build
}//namespace FastFish

#include "Build/Documents/bDocumentsRanked.inl"

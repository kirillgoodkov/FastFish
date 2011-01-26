#pragma once
#include "Containers/Set.h"
#include "Containers/Set2.h"
#include "Memory/ArrayPtr.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

template<typename DOCUMENTS>
class CreateCmprList
{
    CreateCmprList(const CreateCmprList&);
    CreateCmprList& operator = (const CreateCmprList&);

    static const size_t  BlockShift      = FC::PostingsCmprListBlockShift;
    static const size_t  BlockSize       = 1 << BlockShift;
    static const size_t  BlockMask       = BlockSize - 1;
    static const size_t  MinItemsInBlock = BlockSize/5;
    
public:
    typedef Set2<docid_t, MaxDocumentId> SetType;//@@@

    CreateCmprList(const SetType& st, const DOCUMENTS* pDocs) throw();
    
    filepos_t Store(OFStream& file, filepos_t nBase) const ffThrowAll;
    docid_t CountBlocks()                            const throw();
    docid_t Document0()                              const throw();
    
    void operator () (docid_t val) throw();    

    static const size_t MaxItemSize = 1 + (2*BlockSize)/MinItemsInBlock;    
private:    
                                         
    const DOCUMENTS*const   c_pDocs;
    const size_t            c_nBlocks;
    const ArrayPtr<uns1_t>  c_arrStream;    
    uns1_t*const            c_pBegin;
    const ArrayPtr<docid_t> c_arrIndex0;
    
    uns1_t*     m_pNext;
    uns1_t*     m_pNextBlock;
    docid_t     m_didPrev;
    docid_t*    m_pIndex0;
};

}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/Visitors/CreateCmprList.inl"

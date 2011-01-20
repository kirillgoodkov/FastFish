#pragma once
#include "Containers/Deque.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

template<typename DOCUMENTS, bool bCTRL, bool bSENS, TextIndexType tit> 
class PostingsBag
{
    PostingsBag(const PostingsBag&);
    PostingsBag& operator = (const PostingsBag&);
public:
    typedef ReorderBFS<bCTRL, bSENS, tit> BFS;

    PostingsBag(const BFS& bfs, const DOCUMENTS* pDocs) throw();

    void Store(OFStream& file) ffThrowAll;

private:    
    template<typename IDXA_TYPE>
    void StoreIdxA(OFStream& file) const ffThrowAll;

    struct IndexRef
    {
        size_t nNode;
        size_t nIdxB;
    };

    typedef Deque<IndexRef,  PC::FieldTextDeqIdxCmprShift> DeqIdxIr;
    typedef Deque<docid_t,   PC::FieldTextDeqIdxBShift>    DeqIdxB;
    typedef Deque<size_t,    PC::FieldTextDeqIdxAShift>    DeqIdxA;

    const BFS&              c_bfs;
    const DOCUMENTS*const   c_pDocs;
    const docid_t           c_nBmpStartFrom;    
    
    DeqIdxIr        m_arrDeqFixed[FC::PostingsFixedListMaxSize - FC::PostingsInPlace];
    DeqIdxIr        m_arrDeqCmpr[FC::PostingsCmprListMaxShift - FC::PostingsCmprListMinShift];
    DeqIdxIr        m_deqBmp;
    DeqIdxB         m_deqIdxB;
    DeqIdxA         m_deqIdxA;    
};
    

}//namespace Build
}//namespace FastFish

#include "Tools/OFStream.h"
#include "Tools/Bits.h"

namespace FastFish{
namespace Build{

template<typename DOCUMENTS> 
CreateCmprList<DOCUMENTS>::CreateCmprList(const SetType& st, const DOCUMENTS* pDocs) throw():
    c_pDocs(pDocs),
    c_nBlocks((st.Count() + MinItemsInBlock - 1)/MinItemsInBlock),
    c_arrStream(c_nBlocks*(BlockSize + 1) - 1),
    c_pBegin((uns1_t*)(size_t(c_arrStream.Get() + BlockSize - 1) & ~BlockMask)),
    c_arrIndex0((st.Count() + MinItemsInBlock - 1) / MinItemsInBlock),
    m_pNext(c_pBegin),
    m_pNextBlock(c_pBegin + BlockSize),
    m_didPrev(docid_t(pDocs->Count())),
    m_pIndex0(const_cast<docid_t*>(c_arrIndex0.Get()))
{
    *m_pIndex0++ = m_didPrev;    
    st.Enum(*this);
}

template<typename DOCUMENTS> ffForceInline
void CreateCmprList<DOCUMENTS>::operator () (docid_t did) throw()
{
    did = c_pDocs->TransformForw(did);
    ffAssume(did < m_didPrev);
    
    uns1_t* pPrev = m_pNext;
    VbeWrite(m_didPrev - did, m_pNext);    
    
    if (m_pNextBlock <= m_pNext)
    {
        if (m_pNextBlock == m_pNext)
        {
            *m_pIndex0++ = did;    
        }
        else
        {
            ffAssume(pPrev < m_pNextBlock);
            memset(pPrev, 0, m_pNextBlock - pPrev);
            m_pNext = m_pNextBlock;
            
            VbeWrite(m_didPrev - did, m_pNext);
            
            *m_pIndex0++ = m_didPrev;
        }
        m_pNextBlock += BlockSize;
    }
    
    m_didPrev = did;             
}

template<typename DOCUMENTS> ffForceInline
docid_t CreateCmprList<DOCUMENTS>::CountBlocks() const throw()
{
    size_t nCount = (m_pNext - c_pBegin + BlockSize - 1)/BlockSize;
    ffAssume(nCount < docid_t(-1));
    return docid_t(nCount);
}

template<typename DOCUMENTS> ffForceInline
docid_t CreateCmprList<DOCUMENTS>::Document0() const throw()
{
    const uns1_t* p = c_pBegin;
    return docid_t(c_pDocs->Count()) - VbeRead(p);
}

template<typename DOCUMENTS> 
filepos_t CreateCmprList<DOCUMENTS>::Store(OFStream& file, filepos_t nBase) const ffThrowAll
{
    size_t nBlocks = CountBlocks();
    file.Write(c_arrIndex0.Get(), sizeof(docid_t) * nBlocks);
    size_t nRest = (nBlocks * sizeof(docid_t)) & BlockMask;
    if (nRest)
    {
        file.WriteNulls(BlockSize - nRest);
    }        
    
    ffAssert(0 == (file.GetBagPos() - nBase) % BlockSize);
    filepos_t nRet = (file.GetBagPos() - nBase)/BlockSize;
    
    file.Write(c_pBegin, m_pNext - c_pBegin);
    file.WriteNulls((m_pNextBlock - m_pNext) & BlockMask);
    
    return nRet;
}

}//namespace Build
}//namespace FastFish

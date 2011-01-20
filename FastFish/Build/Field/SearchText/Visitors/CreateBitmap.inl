#include "Tools/OFStream.h"
#include "Tools/Bits.h"

namespace FastFish{
namespace Build{

template<typename DOCUMENTS> 
CreateBitmap<DOCUMENTS>::CreateBitmap(const DOCUMENTS* pDocs) throw():
    c_pDocs(pDocs),
    c_nSize(pDocs->Count()/ffBitSizeOf(bmpword_t) + 1),
    m_arr(c_nSize),
    m_nStore(ffCountOf(arrDocuments))
{
    memset(m_arr.Get(), 0, c_nSize * sizeof(bmpword_t));
    SetBit(m_arr.Get(), pDocs->Count());//stop mark for FindBit
}

template<typename DOCUMENTS> 
OFStream& operator << (OFStream& file, const CreateBitmap<DOCUMENTS>& vis) ffThrowAll
{
    file.Write(vis.m_arr.Get(), vis.c_nSize * sizeof(bmpword_t));    
    return file;
}

template<typename DOCUMENTS> ffForceInline
void CreateBitmap<DOCUMENTS>::operator () (docid_t val) throw()
{
    val = c_pDocs->TransformForw(val);
    ffAssert(val/ffBitSizeOf(bmpword_t) < c_nSize);
    SetBit(m_arr.Get(), c_pDocs->Count() - val - 1);
    if (m_nStore)
    {
        arrDocuments[ffCountOf(arrDocuments) - m_nStore] = val;
        --m_nStore;
    }
}


}//namespace Build
}//namespace FastFish

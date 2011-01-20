#include "Search/Field/DataVar/sFieldDataVar.h"
#include "Search/Layers.h"
#include "Tools/IFStream.h"
#include "Tools/Bits.h"
#include "Tools/Tools.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Search{

template<typename POS_TYPE, typename SIZE_TYPE>
FieldDataVar::SegmentT<POS_TYPE, SIZE_TYPE>::SegmentT(IFStream& ifs) throw():
    m_pData(ifs.GetBag<byte_t>(liData)),
    m_pIdx(ifs.GetBag<Block>(liData, sizeof(Block)))
{
}

template<typename POS_TYPE, typename SIZE_TYPE>
const void* FieldDataVar::SegmentT<POS_TYPE, SIZE_TYPE>::Get(docid_t did, datasize_t* pSizeRet) const throw()
{
    const Block& block = m_pIdx[did / BlockSize];
    did %= BlockSize;
    
    const SIZE_TYPE* pSize = block.arrSizes + did;
#ifdef ffDebug
    if (0 == *pSize)
    {
        return 0;
    }
#endif    
    
    POS_TYPE nPos = block.nBase;
    for (const SIZE_TYPE* p = block.arrSizes; p < pSize; ++p)
    {
        nPos += *p;
    }

    if (pSizeRet)
    {
        *pSizeRet = *pSize;
    }
    
    return m_pData + nPos;
}

//----------------------------------------------------------------------------

FieldDataVar::~FieldDataVar() throw()
{
    for (vector<Segment*>::iterator it = m_vecSeg.begin(); it != m_vecSeg.end(); ++it)
    {
        delete *it;
    }
}

void FieldDataVar::Load(IFStream& ifs) throw()
{
    uns1_t nVer; ifs >> nVer;
    CheckVersion(nVer, 0);
    
    uns1_t nSizeOfPos, nSizeOfSize;
    ifs >> nSizeOfPos
        >> nSizeOfSize;
         
    if (4 == nSizeOfPos)         
    {
        if (1 == nSizeOfSize)
        {
            m_vecSeg.push_back(new SegmentT<uns4_t, uns1_t>(ifs));    
        }
        else
        {
            m_vecSeg.push_back(new SegmentT<uns4_t, uns2_t>(ifs));    
        }
    }
    else
    {
        if (1 == nSizeOfSize)
        {
            m_vecSeg.push_back(new SegmentT<uns8_t, uns1_t>(ifs));    
        }
        else
        {
            m_vecSeg.push_back(new SegmentT<uns8_t, uns2_t>(ifs));    
        }
    }
}

const void* FieldDataVar::Get(Document doc, datasize_t* pSize) const throw()
{
    ffAssertUser(doc.m.sid < m_vecSeg.size(), "invalid segment id");
    return m_vecSeg[doc.m.sid]->Get(doc.m.did, pSize);        
}



}//namespace Search
}//namespace FastFish

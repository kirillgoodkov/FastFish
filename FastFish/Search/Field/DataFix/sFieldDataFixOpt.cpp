#include "Search/Field/DataFix/sFieldDataFixOpt.h"
#include "Search/Layers.h"
#include "Tools/IFStream.h"
#include "Tools/Bits.h"
#include "Tools/Tools.h"
#include "Common/Templates.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Search{

FieldDataFixOpt::FieldDataFixOpt(const TplFieldDataFix& tpl) throw():
    c_tpl(tpl)
{
    ffAssume(0 < c_tpl.nSize);
}

void FieldDataFixOpt::Load(IFStream& ifs) throw()
{
    uns1_t nVer; ifs >> nVer;
    CheckVersion(nVer, 0);

    m_vecSeg.push_back(Segment());    
    Segment& seg  = m_vecSeg.back();
    ifs >> seg.nCount;
    size_t nAlign = min(MaxLessOrEqualPow2(size_t(c_tpl.nSize)), FC::DataFixMaxAlign);
    seg.pData = ifs.GetBag<byte_t>(liData, nAlign);
    seg.pIdx  = ifs.GetBag<docid_t>(liData, 2 * sizeof(docid_t));
}

const void* FieldDataFixOpt::Get(Document doc, datasize_t* pSize) const throw()
{
    ffAssertUser(doc.m.sid < m_vecSeg.size(), "invalid segment id");
    const Segment& seg = m_vecSeg[doc.m.sid];
    ffAssumeUser(doc.m.did < seg.nCount, "invalid document id");
    
    const docid_t* pIdx = seg.pIdx + 2 * (doc.m.did/ffBitSizeOf(docid_t));    
    docid_t didRest     = doc.m.did % ffBitSizeOf(docid_t);
    docid_t nBits       = *pIdx;
    if (nBits & (1 << didRest))
    {
        docid_t nMask = (1 << didRest) - 1;
        ++pIdx;
        ffAssume(docid_t(-1) != *pIdx);        
        docid_t nIdx  = *pIdx + CountBits(nBits & nMask);

        if (pSize)
            *pSize = c_tpl.nSize;
        
        return static_cast<const void*>(seg.pData + nIdx*c_tpl.nSize);            
    }
    else
    {
        if (pSize)
            *pSize = 0;
            
        return 0;    
    }    
}

}//namespace Search
}//namespace FastFish

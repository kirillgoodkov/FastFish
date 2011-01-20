#include "Search/Field/DataFix/sFieldDataFix.h"
#include "Search/Layers.h"
#include "Tools/IFStream.h"
#include "Tools/Bits.h"
#include "Tools/Tools.h"
#include "Common/Templates.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Search{

FieldDataFix::FieldDataFix(const TplFieldDataFix& tpl) throw():
    c_tpl(tpl)
{
    ffAssume(0 < c_tpl.nSize);
}

void FieldDataFix::Load(IFStream& ifs) throw()
{
    uns1_t nVer; ifs >> nVer;
    CheckVersion(nVer, 0);

    m_vecSeg.push_back(Segment());    
    Segment& seg  = m_vecSeg.back();
    ifs >> seg.nCount;
    size_t nAlign = min(MaxLessOrEqualPow2(size_t(c_tpl.nSize)), FC::DataFixMaxAlign);
    seg.pData = ifs.GetBag<byte_t>(liData, nAlign);
}

const void* FieldDataFix::Get(Document doc, datasize_t* pSize) const throw()
{
    ffAssert(doc.m.sid < m_vecSeg.size());
    ffAssertUser(doc.m.did < m_vecSeg[doc.m.sid].nCount, "invalid document id");
    if (pSize)
    {
        *pSize = c_tpl.nSize;
    }
    
    return static_cast<const void*>(m_vecSeg[doc.m.sid].pData + doc.m.did*c_tpl.nSize);
}


}//namespace Search
}//namespace FastFish

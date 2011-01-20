#include "Build/Field/DataFix/bFieldDataFix.h"
#include "Build/Documents/bDocumentsNoRanks.h"
#include "Build/Documents/bDocumentsRanked.h"
#include "Tools/Bits.h"
#include "Tools/OFStream.h"
#include "Tools/Tools.h"
#include "Common/Templates.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Build{


FieldDataFix::FieldDataFix(const TplIndex& tplIndex, const TplFieldDataFix& tpl, const Documents* pDocs) throw():
    c_tplIndex(tplIndex),
    c_pDocs(pDocs),
    m_deqData(tpl.nSize)
{
    ffDebugOnly(m_bAdded = false);
}

void FieldDataFix::Add(const void* pData, datasize_t nSize) throw()
{
    ffTouch(nSize);
    ffAssumeUser(0 == nSize, "invalid argument nSize");
    ffAssertUser(!m_bAdded, "value added twice");
    m_deqData.PushBack(pData);    
    ffDebugOnly(m_bAdded = true);
    
}

void FieldDataFix::Commit(rank_t /*rank*/) throw()
{
    ffAssert(c_pDocs->Count() == m_deqData.Count() - 1);
    ffAssertUser(m_bAdded, "mandatory Field is empty");
    ffDebugOnly(m_bAdded = false);
}

template<typename DOCUMENTS> ffForceInline
void FieldDataFix::StoreT(OFStream& file, const DOCUMENTS* pDocs) const ffThrowAll
{
    file << uns1_t(0);//version
    file << docid_t(m_deqData.Count());
    file.BagBegin(min(MaxLessOrEqualPow2(m_deqData.ItemSize()), FC::DataFixMaxAlign));
    for (docid_t n = 0; n < pDocs->Count(); ++n)
    {
        file.Write(m_deqData[pDocs->TransformBack(n)], m_deqData.ItemSize());
    }
    file.BagCommit();
    
    ffLog(lsInfoFileSummary, "\tfix:      "); file.DumpSize();
}

void FieldDataFix::Store(OFStream& file) const ffThrowAll
{
    if (1 == c_tplIndex.nRanksCount)
    {
        StoreT(file, down_cast<const DocumentsNoRanks*>(c_pDocs));
    }
    else
    {
        StoreT(file, down_cast<const DocumentsRanked*>(c_pDocs));
    }
}


}//namespace Build
}//namespace FastFish


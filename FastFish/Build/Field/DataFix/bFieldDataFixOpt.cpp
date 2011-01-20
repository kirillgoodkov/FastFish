#include "Build/Field/DataFix/bFieldDataFixOpt.h"
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

FieldDataFixOpt::FieldDataFixOpt(const TplIndex& tplIndex, const TplFieldDataFix& tpl, const Documents* pDocs) throw():
    c_tplIndex(tplIndex),
    c_pDocs(pDocs),
    m_deqData(tpl.nSize),
    m_bAdded(false)
{
}

void FieldDataFixOpt::Add(const void* pData, datasize_t nSize) throw()
{
    ffTouch(nSize);
    ffAssumeUser(0 == nSize, "invalid argument nSize");
    if (!m_bAdded)
    {
        m_deqData.PushBack(pData);    
        m_bAdded = true;
    }
}

void FieldDataFixOpt::Commit(rank_t) throw()
{
    ffAssert(c_pDocs->Count() == m_deqIdx.Count());
    if (m_bAdded)
    {
        m_deqIdx.PushBack(docid_t(m_deqData.Count() - 1));
        m_bAdded = false;
    }    
    else
    {
        m_deqIdx.PushBack(docid_t(-1));
    }
}

template<typename DOCUMENTS> ffForceInline
void FieldDataFixOpt::StoreData(OFStream& file, const DOCUMENTS* pDocs, docid_t* arrIdxNew) const ffThrowAll
{
    file.BagBegin(min(MaxLessOrEqualPow2(m_deqData.ItemSize()), FC::DataFixMaxAlign));
    docid_t nPosNew = 0;
    for (docid_t n = 0; n < m_deqIdx.Count(); ++n)
    {
        docid_t nPos = m_deqIdx[pDocs->TransformBack(n)];
        if (docid_t(-1) != nPos)
        {
            file.Write(m_deqData[nPos], m_deqData.ItemSize());
            arrIdxNew[n] = nPosNew++;
        }
        else
        {
            arrIdxNew[n] = docid_t(-1);
        }
    }
    file.BagCommit();
    ffLog(lsInfoFileSummary, "\tfix.data: "); file.DumpSize();        
}

void FieldDataFixOpt::Store(OFStream& file) const ffThrowAll
{
    ffAssert(c_pDocs->Count() == m_deqIdx.Count());
    
    file << uns1_t(0);//version        
    file << docid_t(m_deqIdx.Count());
    
    ArrayPtr<docid_t> arrIdxNew(m_deqIdx.Count());

    if (1 == c_tplIndex.nRanksCount)
        StoreData(file, down_cast<const DocumentsNoRanks*>(c_pDocs), arrIdxNew.Get());
    else
        StoreData(file, down_cast<const DocumentsRanked*>(c_pDocs), arrIdxNew.Get());

    file.BagBegin(2 * sizeof(docid_t));
    for (docid_t nBase = 0; nBase < m_deqIdx.Count(); nBase += ffBitSizeOf(docid_t))
    {
        docid_t nBits    = 0;
        docid_t nPosBase = docid_t(-1);
        docid_t nCount   = min(nBase + docid_t(ffBitSizeOf(docid_t)), docid_t(m_deqIdx.Count()));
        for (docid_t n = nBase; n < nCount; ++n)
        {
            docid_t nPosCur = arrIdxNew[n];
            if (docid_t(-1) != nPosCur)
            {
                nBits |= 1 << (n - nBase);
                if (docid_t(-1) == nPosBase)
                {
                    nPosBase = nPosCur;
                }
            }
        }
        
        file << nBits << nPosBase;
    }
    file.BagCommit();
    ffLog(lsInfoFileSummary, "\tfix.idx:  "); file.DumpSize();            
}

}//namespace Build
}//namespace FastFish


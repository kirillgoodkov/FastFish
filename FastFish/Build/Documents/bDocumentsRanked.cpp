#include "Build/Documents/bDocumentsRanked.h"
#include "Tools/OFStream.h"
#include "Common/Templates.h"

namespace FastFish{
namespace Build{

DocumentsRanked::DocumentsRanked(const TplIndex& tpl) throw():
    c_tpl(tpl),
    m_a(PC::DocumentsAllocMul * (1 << PC::DocumentsDeqShift) * sizeof(docid_t), 
                                    (1 << PC::DocumentsDeqShift) * sizeof(docid_t),
                                                                       sizeof(docid_t)),
    m_arrLists(c_tpl.nRanksCount),
    m_didCount(0),
    m_nRanks(0)
{
    ffAssume(1 < c_tpl.nRanksCount);
}

docid_t DocumentsRanked::Add(rank_t rank) throw()
{
    ffAssume(rank < c_tpl.nRanksCount);
    m_arrLists[rank].PushBack(m_didCount, m_a);
    return m_didCount++;
}

void DocumentsRanked::Finalize() throw()
{
    m_arrIndexForw.Alloc(m_didCount);
    m_arrIndexBack.Alloc(m_didCount);
    m_arrIndexRanks.Alloc(c_tpl.nRanksCount);
    
    docid_t didNew = m_didCount - 1;
    for (size_t nRank = c_tpl.nRanksCount; nRank; --nRank)
    {
        m_arrIndexRanks[nRank - 1] = didNew;
        const DequeDocuments& deq = m_arrLists[nRank - 1];
        m_nRanks += (0 == deq.Count()) ? 0 : 1;
        for (size_t n = deq.Count(); n; --n, --didNew)
        {
            m_arrIndexForw[deq[n - 1]] = didNew;
            m_arrIndexBack[didNew] = deq[n - 1];
        }
    }
    ffAssume(docid_t(-1) == didNew);
    
    m_a.Clear();
    m_arrLists.Clear();
}

void DocumentsRanked::Store(OFStream& file) const ffThrowAll
{
    ffAssert(!m_arrIndexRanks.IsEmpty());
    file << uns1_t(0)//version                              
         << m_didCount
         << m_nRanks;

    file.BagBegin(sizeof(docid_t));
    file.Write(m_arrIndexRanks.Get(), c_tpl.nRanksCount * sizeof(docid_t));
    file.BagCommit();
         
    file.BagBegin(sizeof(docid_t));
    docid_t did = m_arrIndexRanks[0];
    file << did;
    for (const docid_t* p = m_arrIndexRanks.Get() + 1; p < m_arrIndexRanks.Get() + c_tpl.nRanksCount; ++p)
    {       
        if (did != *p)
        {
            did = *p;
            file << did;
        }
    }
    file.BagCommit();
    
    file.BagBegin(sizeof(rank_t));
    file << rank_t(0);
    const docid_t* p = m_arrIndexRanks.Get();
    for (rank_t rank = 1; rank < c_tpl.nRanksCount; ++rank)
    {
        if (*p != m_arrIndexRanks[rank])
        {
            p = m_arrIndexRanks.Get() + rank;
            file << rank;
        }        
    }        
    file.BagCommit();
    
    ffLog(lsInfoFileSummary, "docs:\t          "); file.DumpSize();
}

}//namespace Build
}//namespace FastFish


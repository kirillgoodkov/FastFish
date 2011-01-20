#include "Search/Documents/sDocuments.h"
#include "Search/Layers.h"
#include "Tools/IFStream.h"
#include "Tools/Bits.h"
#include "Tools/Tools.h"

using namespace std;

namespace FastFish{
namespace Search{

Documents::Documents(const TplIndex& tpl) throw():
    c_tpl(tpl),
    m_nTotalCount(0)
{
}

void Documents::Load(IFStream& ifs) ffThrowNone
{
    ffAssert(m_vecSegments.size() < MaxSegId);

    uns1_t nVer; ifs >> nVer;
    CheckVersion(nVer, 0);
    
    Segment seg;
    ifs >> seg.nCount
        >> seg.nCountRanks;

    ffAssume(0 < seg.nCount && seg.nCount <= MaxDocumentId + 1);
    
    seg.pRank2Did = ifs.GetBag<docid_t>(liDocuments, sizeof(docid_t));
    seg.pDids     = ifs.GetBag<docid_t>(liDocuments, sizeof(docid_t));
    seg.pRanks    = ifs.GetBag<rank_t>(liDocuments, sizeof(rank_t));
    
    m_vecSegments.push_back(seg);
    m_nTotalCount += seg.nCount;

    vector<Document> vecRev;
    vecRev.reserve(seg.nCountRanks);
    for (size_t n = seg.nCountRanks; n; --n)
    {
        Document doc;
        doc.m.rank = seg.pRanks[n - 1];
        doc.m.sid  = segid_t(m_vecSegments.size() - 1);
        doc.m.did  = (1 < n) ? (seg.pDids[n - 2] + 1) : 0;
        vecRev.push_back(doc);
    }
    
    vector<Document> vecRanked(m_vecRanked.size() + seg.nCountRanks);
    merge(vecRev.begin(), vecRev.end(), m_vecRanked.begin(), m_vecRanked.end(), vecRanked.begin(), CmpRS());
    
    m_vecRanked.swap(vecRanked);
}

}//namespace Search
}//namespace FastFish

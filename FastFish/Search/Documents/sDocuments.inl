#include "Common/TemplateLoaders.h"

namespace FastFish{
namespace Search{

inline
rank_t Documents::GetRank(segid_t sid, docid_t did) const throw() 
{
    const Segment& seg = m_vecSegments[sid];
    return seg.pRanks[std::lower_bound(seg.pDids, seg.pDids + seg.nCountRanks, did) - seg.pDids];
}

inline
docid_t Documents::GetDid(segid_t sid, rank_t rank) const throw()
{
    ffAssume(rank < c_tpl.nRanksCount);
    return m_vecSegments[sid].pRank2Did[rank];
    
}

inline
Documents::Context Documents::GetFirst(Document& doc) const throw()
{
    doc = m_vecRanked.front();
    doc.m.did = m_vecSegments[doc.m.sid].nCount - 1;
    return &m_vecRanked.front();
}

inline
bool Documents::GetNext(Context& ctx, Document& doc) const throw()
{
    if (doc == *ctx)
    {
        if (ctx == &m_vecRanked.back())
        {
            return false;            
        }        
        doc = *++ctx;
        doc.m.did = m_vecSegments[doc.m.sid].pRank2Did[doc.m.rank];
    }
    else
    {
        --doc.m.did;
    }
    
    return true;        
}

}//namespace Search
}//namespace FastFish

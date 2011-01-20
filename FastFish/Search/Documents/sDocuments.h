#pragma once
#include "Tools/Tools.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class Documents
{
    Documents(const Documents&);
    Documents& operator = (const Documents&);
public:
    Documents(const TplIndex& tpl)                      throw();

    void     Load(IFStream& ifs)                        ffThrowNone;
    
    size_t   Count()                              const throw()     {return m_nTotalCount;}        
    rank_t   GetRank(segid_t sid, docid_t did)    const throw();
    docid_t  GetDid(segid_t sid, rank_t rank)     const throw();

    typedef const Document* Context;    
    Context  GetFirst(Document& doc)              const throw();
    bool     GetNext(Context& ctx, Document& doc) const throw();
    
private:        
    struct Segment
    {
        docid_t         nCount;
        const docid_t*  pRank2Did;        
        
        rank_t          nCountRanks;
        const docid_t*  pDids;
        const rank_t*   pRanks;
    };

    const TplIndex&         c_tpl;
    
    size_t                  m_nTotalCount;
    std::vector<Segment>    m_vecSegments;
    std::vector<Document>   m_vecRanked;

//ifdef only for optimization, both variants are correct
#ifdef ff32
    struct CmpRS
    {
        bool operator ()(Document a, Document b) const throw() {return CompareRS(b, a);}
    };
#else
    typedef std::greater<Document> CmpRS;
#endif        
    
};

}//namespace Search
}//namespace FastFish

#include "Search/Documents/sDocuments.inl"

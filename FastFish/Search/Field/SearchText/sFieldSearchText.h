#pragma once
#include "Search/Field/sField.h"
#include "Search/Field/SearchText/sSuffixTree.h"
#include "Search/Field/SearchText/sPostingsBag.h"
#include "Memory/AllocatorInvaderBB.h"
#include "Memory/AllocatorInvader.h"
#include "Memory/ArrayPtr.h"
#include "Tools/Strings.h"              
#include "Common/Common.h"

namespace FastFish{
namespace Search{

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit>
class FieldSearchText:
    public FieldSearch
{
    FieldSearchText(const FieldSearchText&);
    FieldSearchText& operator = (const FieldSearchText&);
public:
    FieldSearchText(const TplFieldSearchText& tpl, const Documents* pDoc)       throw();
    virtual ~FieldSearchText()                                                  throw()     {}
                                                                            
    virtual void    Load(IFStream& ifs)                                         throw();
                                                                            
    virtual bool    FindPostings(handle_t arr[], size_t& nCount, void* arg)     throw();
    virtual void    PsFree(handle_t h)                                          throw();
    virtual void    PsReset(handle_t h)                                         throw()     {static_cast<Postings*>(h)->ResetCursors();}

    virtual bool    PsGetNext(handle_t h, Document& doc)                        throw();
    virtual bool    PsGetNextFrom(handle_t h, Document docFrom, Document& doc)  throw();
    
    virtual bool    PsIsContain(handle_t h, Document doc)                       throw();
    virtual size_t  PsGetCardinality(handle_t h)                          const throw()     {return static_cast<const Postings*>(h)->nCardinality;}
    
    
private:   
    Document GetNextDocument(Postings* pPs) throw();
    
    template <bool bUpdateRank>
    void   InsertCursorFrom(Postings* pPs, PsCursor cur, Document docFrom) throw();

    struct Segment
    {
        SuffixTree<titExact == tit, bEMPTYCHECK>  st;
        PostingsBag                               pb;
        
        Segment(IFStream& ifs, const Documents* pDoc) throw(): st(ifs), pb(ifs, pDoc) {};        
    };


    const TplFieldSearchText&       c_tpl;
    const StrCSpn                   c_cspn;    
    const Documents*const           c_pDocs;

    std::vector<Segment>            m_vecSeg;
    std::vector<char>               m_vecStrTemp;
    std::auto_ptr<AllocatorSwapper> m_pAllocPostings;    
    
    mutable ArrayPtr<PsCursor>      m_arrCurTemp;     
};

}//namespace Search
}//namespace FastFish


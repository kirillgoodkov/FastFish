#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

enum OptNodeTypes
{
    ontAnd,
    ontOr,
    ontLeaf,
    ontLeafNeg,
    ontFull,
    ontEmpty,        
    
    ontsContainer = ontOr + 1
};

struct OptNodeAndOr;
struct SeqNodeD;
struct SeqNodeL;

struct OptNode
{
    OptNodeTypes    m_ont;
    bool            m_bListAccept;
    bool            m_bList;
    real_t          m_rCardinality;
    
    void        TransformDropEF()                                          throw();//bottom->top
    OptNode*    TransformMergeDomains(AllocatorInvaderBB& a)               throw();//top->bottom
    void        TransformSort(real_t rTotal)                               throw();//bottom->top
    void        TransformSetLD(bool bList)                                 throw();//top->bottom
    
    size_t      DomainSize(OptNodeTypes ontDomain)                   const throw();
    void        DomainEnum(OptNodeAndOr* pNodeRoot, AllocatorInvaderBB& a) throw();
    
    size_t      SeqTreeEstimate()                                    const throw();
    SeqNodeL*   SeqTreeCreateL(byte_t*& pMem, Documents* pDocs)      const throw();
    SeqNodeD*   SeqTreeCreateD(byte_t*& pMem)                        const throw();
    
    void        Dump(LogStream ls)                                   const throw();         
    bool        IsContainer()                                        const throw() {return m_ont < ontsContainer;}
    
    friend bool operator < (const OptNode& a, const OptNode& b) throw() {return a.m_rCardinality < b.m_rCardinality;}
    friend bool operator > (const OptNode& a, const OptNode& b) throw() {return a.m_rCardinality > b.m_rCardinality;}    
};

struct OptNodeAndOr: public OptNode
{
    size_t      m_nCount;          
    OptNode*    m_arrChilds[1];
    
    OptNode*const* End() const throw()          {return m_arrChilds + m_nCount;};
    OptNode**      End() throw()                {return m_arrChilds + m_nCount;};
    
    static size_t SizeOf(size_t nCount) throw() {return sizeof(OptNodeAndOr) + sizeof(void*)*(nCount - 1);}        
};

struct OptNodeLeaf: public OptNode
{
    FieldSearch*    m_pField;
    handle_t        m_hPostings;        
};

}//namespace Search
}//namespace FastFish

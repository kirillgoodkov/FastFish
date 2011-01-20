#include "Search/Query/OptNodes.h"
#include "Search/Query/SeqNodes.h"
#include "Search/Field/sField.h"
#include "Memory/AllocatorInvaderBB.h"
#include "Tools/Tools.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Search{

void OptNode::TransformDropEF() throw()
{
    if (IsContainer())
    {
        OptNodeAndOr* pThis = static_cast<OptNodeAndOr*>(this);              

        OptNode** ppDst = pThis->m_arrChilds;
        for (OptNode** ppSrc = pThis->m_arrChilds; ppSrc < pThis->End(); ++ppSrc)
        {
            (*ppSrc)->TransformDropEF();
            switch ((*ppSrc)->m_ont)
            {
                case ontEmpty: 
                    if (ontAnd == m_ont)
                    {
                        pThis->m_ont = ontEmpty; 
                        return;                            
                    }
                    break;
                case ontFull:                           
                    if (ontOr == m_ont)
                    {
                        pThis->m_ont = ontFull; 
                        return;
                    }
                    break;
                default:       
                    *ppDst++ = *ppSrc;
            }
        }
        pThis->m_nCount = ppDst - pThis->m_arrChilds;            
        if (0 == pThis->m_nCount)
        {
            pThis->m_ont = (ontAnd == m_ont) ? ontFull : ontEmpty;
        }                
    }
} 

OptNode* OptNode::TransformMergeDomains(AllocatorInvaderBB& a) throw()
{
    if (!IsContainer())
        return this;
    
    size_t nCount = DomainSize(m_ont);
    ffAssume(0 != nCount);
    if (1 == nCount)
    {
        OptNodeAndOr nodeTemp;
        nodeTemp.m_ont    = m_ont;
        nodeTemp.m_nCount = 0;
        DomainEnum(&nodeTemp, a);
        return nodeTemp.m_arrChilds[0];            
    }
    else
    {
        OptNodeAndOr* pNodeNew = static_cast<OptNodeAndOr*>(a.Alloc(OptNodeAndOr::SizeOf(nCount)));
        pNodeNew->m_ont    = m_ont;
        pNodeNew->m_nCount = 0;                
        DomainEnum(pNodeNew, a);
        return pNodeNew;
    }
}

size_t OptNode::DomainSize(OptNodeTypes ontDomain) const throw()
{
    if (m_ont != ontDomain)
        return 0;
        
    ffAssume(ontAnd == ontDomain || ontOr == ontDomain);
    
    const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
    size_t nSize = pThis->m_nCount;
    for (OptNode*const* pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
    {
        nSize += (*pp)->DomainSize(ontDomain);
    }
    
    return nSize;
}

void OptNode::DomainEnum(OptNodeAndOr* pNodeRoot, AllocatorInvaderBB& a) throw()
{
    if (m_ont == pNodeRoot->m_ont)
    {
        ffAssume(ontAnd == m_ont || ontOr == m_ont);
        
        const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
        for (OptNode*const* pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
        {
            (*pp)->DomainEnum(pNodeRoot, a);
        }
    }
    else
    {
        pNodeRoot->m_arrChilds[pNodeRoot->m_nCount++] = TransformMergeDomains(a);
    }
}

void OptNode::TransformSort(real_t rTotal) throw()
{
    if (IsContainer())
    {
        OptNodeAndOr* pThis = static_cast<OptNodeAndOr*>(this);
        for (OptNode** pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
        {
            (*pp)->TransformSort(rTotal);
        }            
    }

    switch (m_ont)
    {
        case ontAnd:
        {
            OptNodeAndOr* pThis = static_cast<OptNodeAndOr*>(this);
            sort(pThis->m_arrChilds, pThis->End(), PtrCmp<OptNode, less<OptNode> >);
            m_rCardinality = 1.0;
            m_bListAccept  = pThis->m_arrChilds[0]->m_bListAccept;
            for (OptNode** pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
            {
                pThis->m_rCardinality *= (*pp)->m_rCardinality;
            }                    
            break;        
        }            
        case ontOr:
        {
            OptNodeAndOr* pThis = static_cast<OptNodeAndOr*>(this);
            sort(pThis->m_arrChilds, pThis->End(), PtrCmp<OptNode, greater<OptNode> >);
            m_bListAccept   = true;
            m_rCardinality  = 0.0;
            real_t rCrdRest = 1.0;
            for (OptNode** pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
            {
                real_t r = rCrdRest * (*pp)->m_rCardinality;
                m_rCardinality += r;
                rCrdRest       -= r;
                
                if (!(*pp)->m_bListAccept)
                    m_bListAccept = false;
            }        
            break;
        }
        case ontLeaf:
        case ontLeafNeg:
        {
            const OptNodeLeaf* pThis = static_cast<const OptNodeLeaf*>(this);
            real_t rCrd = real_t(pThis->m_pField->PsGetCardinality(pThis->m_hPostings));
            ffAssume(rCrd <= rTotal);
            if (ontLeafNeg == m_ont)
            {
                rCrd = rTotal - rCrd;
            }
            m_rCardinality = rCrd/rTotal;            
            m_bListAccept  = (ontLeaf == m_ont);
            break;
        }        
        case ontFull:
            m_rCardinality = 1.0;
            m_bListAccept  = true;
            break;
        case ontEmpty:
            m_rCardinality = 0.0;
            m_bListAccept  = true;
            break;
        default:
            ffAssume(false);
    }
}

void OptNode::TransformSetLD(bool bList) throw()
{
    ffAssume(!bList || m_bListAccept);
    m_bList = bList;
    switch (m_ont)
    {
        case ontAnd:
        {
            const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
            if (m_bList)
            {
                real_t rCrdBase = PC::SearcherMergeLimit * pThis->m_arrChilds[0]->m_rCardinality;
                const OptNode*const* ppD = pThis->m_arrChilds + 1;                
                for (; ppD < pThis->End() && 
                        (*ppD)->m_bListAccept && 
                        (*ppD)->m_rCardinality < rCrdBase; 
                       ++ppD)
                {}    
                
                for (OptNode*const* pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
                {
                    (*pp)->TransformSetLD(pp < ppD);
                }
            }
            else
            {
                for (OptNode*const* pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
                {
                    (*pp)->TransformSetLD(false);
                }
            }
            break;
        }    
        case ontOr:
        {
            const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
            for (OptNode*const* pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
            {
                (*pp)->TransformSetLD(m_bList);
            }            
            break;
        }
        default:;
    }
}

size_t OptNode::SeqTreeEstimate() const throw()
{
    switch (m_ont)
    {
        case ontAnd:
        case ontOr:
        {
            const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
            size_t nSize = m_bList ? 
                            ((ontAnd == m_ont) ? 
                                SeqNodeLAnd::SizeOf(pThis->m_nCount) : 
                                SeqNodeLOr::SizeOf(pThis->m_nCount)) : 
                            SeqNodeDAndOr::SizeOf(pThis->m_nCount);
            for (const OptNode*const* pp = pThis->m_arrChilds; pp < pThis->End(); ++pp)
            {
                nSize += (*pp)->SeqTreeEstimate();
            }
            return nSize;
        }    
        case ontLeaf:
            return m_bList ? sizeof(SeqNodeLLeaf) : sizeof(SeqNodeDLeaf);            
        case ontLeafNeg:
            ffAssume(!m_bList);
            return sizeof(SeqNodeDLeaf);            
        case ontFull:
            ffAssume(m_bList);
            return sizeof(SeqNodeLFull);            
        default:
            ffAssume(false);
    }
    ffReturnNop;
}

SeqNodeL* OptNode::SeqTreeCreateL(byte_t*& pMem, Documents* pDocs) const throw()
{
    ffAssume(m_bList);
    switch (m_ont)
    {
        case ontAnd:
        {
            const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
            SeqNodeLAnd*        pNode = AllocPODVar<SeqNodeLAnd>(pMem, pThis->m_nCount);

            OptNode*const* ppOpt = pThis->m_arrChilds;
            SeqNode**      pp    = pNode->arrNodes;
            for (; ppOpt < pThis->End() && (*ppOpt)->m_bList; ++ppOpt, ++pp)
            {
                *pp = (*ppOpt)->SeqTreeCreateL(pMem, pDocs);
            }
            
            pNode->snt     = sntLAnd;
            pNode->doc     = DocumentNull;            
            pNode->nCountL = ppOpt - pThis->m_arrChilds;
            pNode->nCountD = pThis->End() - ppOpt;
            
            for (; ppOpt < pThis->End(); ++ppOpt, ++pp)
            {
                *pp = (*ppOpt)->SeqTreeCreateD(pMem);
            }            
            return pNode;            
        }    
        case ontOr:
        {
            const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
            SeqNodeLOr*         pNode = AllocPODVar<SeqNodeLOr>(pMem, pThis->m_nCount);
            pNode->snt    = sntLOr;
            pNode->doc    = DocumentNull;
            pNode->nCount = pThis->m_nCount;
            
            const OptNode*const* ppOpt = pThis->m_arrChilds;                
            for (SeqNodeL** pp = pNode->arrNodes; ppOpt < pThis->End(); ++ppOpt, ++pp)
            {
                *pp = (*ppOpt)->SeqTreeCreateL(pMem, pDocs);
            }            
            return pNode;
        }
        case ontLeaf:
        {
            const OptNodeLeaf* pThis = static_cast<const OptNodeLeaf*>(this);
            SeqNodeLLeaf*      pNode = AllocPODFix<SeqNodeLLeaf>(pMem);
            pNode->snt       = sntLLeaf;
            pNode->doc       = DocumentNull;
            pNode->pField    = pThis->m_pField;
            pNode->hPostings = pThis->m_hPostings;
            return pNode;
        }            
        case ontFull:
        {
            SeqNodeLFull* pThis = AllocPODFix<SeqNodeLFull>(pMem);
            pThis->snt   = sntLFull;
            pThis->doc   = DocumentNull;
            pThis->pDocs = pDocs;
            pThis->ctx   = 0;
            return pThis;
        }
        default:
            ffAssume(false);
    }
    ffReturnNop;
}

SeqNodeD* OptNode::SeqTreeCreateD(byte_t*& pMem) const throw()
{
    ffAssume(!m_bList);
    switch (m_ont)
    {
        case ontAnd:
        case ontOr:
        {
            const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
            SeqNodeDAndOr*      pNode = AllocPODVar<SeqNodeDAndOr>(pMem, pThis->m_nCount);
            pNode->snt    = (ontAnd == m_ont) ? sntDAnd : sntDOr;
            pNode->nCount = pThis->m_nCount;

            const OptNode*const* ppOpt = pThis->m_arrChilds;                
            for (SeqNodeD** pp = pNode->arrNodes; ppOpt < pThis->End(); ++ppOpt, ++pp)
            {
                *pp = (*ppOpt)->SeqTreeCreateD(pMem);
            }            
            
            return pNode;
        }    
        case ontLeaf:
        case ontLeafNeg:
        {
            const OptNodeLeaf* pThis = static_cast<const OptNodeLeaf*>(this);
            SeqNodeDLeaf*      pNode = AllocPODFix<SeqNodeDLeaf>(pMem);
            pNode->snt       = (ontLeaf == m_ont) ? sntDLeaf : sntDLeafNeg;
            pNode->pField    = pThis->m_pField;
            pNode->hPostings = pThis->m_hPostings;
            return pNode;
        }            
        default:
            ffAssume(false);
    }
    ffReturnNop;
}

void OptNode::Dump(LogStream ls) const throw() //debug
{
    switch (m_ont)
    {
        case ontAnd:
        case ontOr:
        {
            const OptNodeAndOr* pThis = static_cast<const OptNodeAndOr*>(this);
            
            ffLog(ls, "(");
            pThis->m_arrChilds[0]->Dump(ls);
            for (OptNode*const* pp = pThis->m_arrChilds + 1; pp < pThis->End(); ++pp)
            {
                ffLog(ls, " %c ", (ontAnd == m_ont) ? '&' : '|');
                (*pp)->Dump(ls);
            }
            ffLog(ls, ")");
            break;
        } 
        case ontLeaf:
        case ontLeafNeg:
        {
            const OptNodeLeaf* pThis = static_cast<const OptNodeLeaf*>(this);
            ffLog(ls, (ontLeaf == m_ont) ? "%u" : "!%u", pThis->m_pField->PsGetCardinality(pThis->m_hPostings));
            break;
        }             
        case ontFull:
            ffLog(ls, "F");
            break;
        case ontEmpty:
            ffLog(ls, "E");
            break;
        default:
            ffAssume(false);
    }    
}

}//namespace Search
}//namespace FastFish

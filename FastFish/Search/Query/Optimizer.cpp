#include "Search/Query/Optimizer.h"
#include "Search/Query/OptNodes.h"
#include "Search/Query/AstNodes.h"
#include "Search/Query/SeqNodes.h"
#include "Search/Query/Query.h"
#include "Search/Field/sField.h"
#include "Search/Documents/sDocuments.h"
#include "Search/Searcher.h"
#include "Tools/Bits.h"

using namespace std;

namespace FastFish{
namespace Search{

Optimizer::Optimizer(const Query& q, void* arrArgs[]) throw():
    m_q(q),
    m_arrArgs(arrArgs),
    m_a(q.m_srch.m_aOptimizer)
{
    m_pRoot = TransformExpandSop(0);
    
    if (ffIsLogged(lsInfoQueryOriginal))
    {
        m_pRoot->Dump(lsInfoQueryOriginal);
        ffLog(lsInfoQueryOptimized, "\n");    
    }
    
    m_pRoot->TransformDropEF();
    m_pRoot = m_pRoot->TransformMergeDomains(m_a);
    m_pRoot->TransformSort(real_t(m_q.m_srch.m_pDocs->Count()));
    m_pRoot->m_bList = m_pRoot->m_bListAccept;
    m_pRoot->TransformSetLD(m_pRoot->m_bListAccept);

    if (!(m_pRoot->m_bList || m_pRoot->m_ont == ontFull))
    {
        OptNode* pFull = NewPODFromRaw<OptNode>(m_a);
        pFull->m_ont   = ontFull;
        pFull->m_bList = true;
        
        switch (m_pRoot->m_ont)
        {
            case ontAnd:
            {
                const OptNodeAndOr* pOldRoot = static_cast<const OptNodeAndOr*>(m_pRoot);
                OptNodeAndOr*       pNewRoot = static_cast<OptNodeAndOr*>(m_a.Alloc(OptNodeAndOr::SizeOf(pOldRoot->m_nCount + 1)));
                pNewRoot->m_nCount       = pOldRoot->m_nCount + 1;
                pNewRoot->m_ont          = ontAnd;                
                pNewRoot->m_bList        = true;
                pNewRoot->m_arrChilds[0] = pFull;
                memcpy(pNewRoot->m_arrChilds + 1, pOldRoot->m_arrChilds, sizeof(OptNode*) * pOldRoot->m_nCount);
                m_pRoot = pNewRoot;
                break;
            }                
            case ontOr:
            case ontLeafNeg:
            {
                OptNodeAndOr* pNewRoot   = static_cast<OptNodeAndOr*>(m_a.Alloc(OptNodeAndOr::SizeOf(2)));
                pNewRoot->m_nCount       = 2;
                pNewRoot->m_ont          = ontAnd;                
                pNewRoot->m_bList        = true;
                pNewRoot->m_arrChilds[0] = pFull;
                pNewRoot->m_arrChilds[1] = m_pRoot;
                m_pRoot = pNewRoot;
                break;
            }                
            default:
                ffAssume(false);
        }
    }

    if (ffIsLogged(lsInfoQueryOptimized))
    {
        m_pRoot->Dump(lsInfoQueryOptimized);
        ffLog(lsInfoQueryOptimized, "\n");
    }    
}

OptNode* Optimizer::TransformExpandSop(size_t nIdx) throw()
{
    const AstNode* pAstNode = &reinterpret_cast<const AstNode&>(m_q.m_arrNodes[nIdx]);    
    bool bFirst = false;
    switch (pAstNode->GetType())
    {
        case antNodeAnd: bFirst = true;
        case antNodeOr:
        {
            const AstNodeC* pNodeC = static_cast<const AstNodeC*>(pAstNode);
            OptNodeAndOr* pNode    = static_cast<OptNodeAndOr*>(m_a.Alloc(OptNodeAndOr::SizeOf(pNodeC->nLastIdx + 1)));
            pNode->m_ont    = bFirst ? ontAnd : ontOr;
            pNode->m_nCount = pNodeC->nLastIdx + 1;
            for (size_t n = 0; n < pNode->m_nCount; ++n)
            {
                pNode->m_arrChilds[n] = TransformExpandSop(m_q.m_arrShift[pNodeC->arrChilds[n] - 1]);
            }
            return pNode;            
        }
        case antLeafA: bFirst = true;
        case antLeafS:
        {
            const AstLeafSop* pAstLeaf = static_cast<const AstLeafSop*>(pAstNode);
            handle_t arrPostings[MaxWordsInArgument];
            size_t nCount = ffCountOf(arrPostings);
            ffAssertUser(dynamic_cast<FieldSearch*>(m_q.m_srch.m_arrFields[pAstLeaf->nFldId].get()), "this Field type don't support using in query");
            FieldSearch* pFld = static_cast<FieldSearch*>(m_q.m_srch.m_arrFields[pAstLeaf->nFldId].get());
            bool bFindAll = pFld->FindPostings(arrPostings, nCount, m_arrArgs[pAstLeaf->nArgId]);
            bool bNeg     = pAstNode->IsNeg();
            
            if (0 == nCount ||
                (!bFindAll  && (
                    ( bFirst && !bNeg) |
                    (!bFirst &&  bNeg) )))
            {
                for (handle_t* p = arrPostings; p < arrPostings + nCount; ++p)
                {
                    pFld->PsFree(*p);
                }
                OptNode* pNode = NewPODFromRaw<OptNode>(m_a);
                pNode->m_ont = bNeg ? ontFull : ontEmpty;
                return pNode;
            }
            else if (1 == nCount)
            {
                OptNodeLeaf* pLeaf = NewPODFromRaw<OptNodeLeaf>(m_a);
                pLeaf->m_ont       = bNeg ? ontLeafNeg : ontLeaf;
                pLeaf->m_pField    = pFld;
                pLeaf->m_hPostings = arrPostings[0];
                return pLeaf;                
            }
            else
            {
                OptNodeAndOr* pNode = static_cast<OptNodeAndOr*>(m_a.Alloc(OptNodeAndOr::SizeOf(nCount)));
                pNode->m_ont    = bFirst ? ontAnd : ontOr;
                pNode->m_nCount = nCount;
                for (size_t n = 0; n < nCount; ++n)
                {
                    OptNodeLeaf* pLeaf = NewPODFromRaw<OptNodeLeaf>(m_a);
                    pLeaf->m_ont          = bNeg ? ontLeafNeg : ontLeaf;
                    pLeaf->m_pField       = pFld;
                    pLeaf->m_hPostings    = arrPostings[n];
                    pNode->m_arrChilds[n] = pLeaf;
                }
                return pNode;                    
            }
        
        }
        default:
            ffAssume(false);
    }   
    
    ffReturnNop;
}

Sequence* Optimizer::CreateQuery() const ffThrowNone
{
    if (ontEmpty == m_pRoot->m_ont)
    {
        return 0;
    }
    size_t nSize   = m_pRoot->SeqTreeEstimate();
    auto_ptr<uns8_t> p(new uns8_t[Sequence::SizeOf(nSize)/sizeof(uns8_t)]);
    Sequence* pSeq = reinterpret_cast<Sequence*>(p.get());
    byte_t* pMem = reinterpret_cast<byte_t*>(pSeq->m_arrData);
    new (pSeq) Sequence(size_t(m_q.m_srch.m_pDocs->Count() * m_pRoot->m_rCardinality), 
                        m_pRoot->SeqTreeCreateL(pMem, m_q.m_srch.m_pDocs.get()));
    p.release();                        
    return pSeq;
}

}//namespace Search
}//namespace FastFish

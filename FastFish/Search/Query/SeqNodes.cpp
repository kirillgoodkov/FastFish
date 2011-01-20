#include "Search/Query/SeqNodes.h"
#include "Search/Field/sField.h"
#include "Search/Documents/sDocuments.h"

using namespace std;;

namespace FastFish{
namespace Search{

ffForceInline
bool SeqNodeLAnd::Filter() const throw()
{
    for (SeqNodeD*const* pp = BeginD(); pp < EndD(); ++pp)
    {
        if (!(*pp)->Find(doc))
        {
            return false;
        }                        
    }
    return true;
}

void SeqNode::FreePostings() throw()
{
    switch (snt)
    {
        case sntDAnd:
        case sntDOr:
        {
            const SeqNodeDAndOr* pThis = static_cast<const SeqNodeDAndOr*>(this);
            for (SeqNodeD*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                (*pp)->FreePostings();
            }
            break;
        }            
        case sntLAnd:
        {
            const SeqNodeLAnd* pThis = static_cast<const SeqNodeLAnd*>(this);
            for (SeqNode*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                (*pp)->FreePostings();
            }
            break;
        }
        case sntLOr:
        {
            const SeqNodeLOr* pThis = static_cast<const SeqNodeLOr*>(this);
            for (SeqNodeL*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                (*pp)->FreePostings();
            }
            break;        
        }
        case sntDLeaf:
        case sntDLeafNeg:
        {
            const SeqNodeDLeaf* pThis = static_cast<const SeqNodeDLeaf*>(this);
            pThis->pField->PsFree(pThis->hPostings);
            break;
        }
        case sntLLeaf:
        {
            const SeqNodeLLeaf* pThis = static_cast<const SeqNodeLLeaf*>(this);
            pThis->pField->PsFree(pThis->hPostings);
            break;
        }
        case sntLFull:
        {
            break;
        }           
        default:
            ffAssume(false);
    }
}

bool SeqNodeD::Find(const Document& doc) throw() 
{
    switch (snt)
    {
        case sntDAnd:
        {
            SeqNodeDAndOr* pThis = static_cast<SeqNodeDAndOr*>(this);
            for (SeqNodeD*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                if (!(*pp)->Find(doc))
                    return false;
            }            
            return true;
        }            
        case sntDOr:
        {
            SeqNodeDAndOr* pThis = static_cast<SeqNodeDAndOr*>(this);
            for (SeqNodeD*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                if ((*pp)->Find(doc))
                    return true;
            }                        
            return false;
        }            
        case sntDLeaf:        
        {            
            const SeqNodeDLeaf* pThis = static_cast<const SeqNodeDLeaf*>(this);
            return pThis->pField->PsIsContain(pThis->hPostings, doc); 
        }
        case sntDLeafNeg:        
        {            
            const SeqNodeDLeaf* pThis = static_cast<const SeqNodeDLeaf*>(this);
            return !pThis->pField->PsIsContain(pThis->hPostings, doc); 
        }
        default:
            ffAssume(false);
    }
    ffReturnNop;
}

void SeqNodeL::Pop(Document docFrom) throw()
{
    switch (snt)
    {
        case sntLAnd:
        {
            SeqNodeLAnd* pThis = static_cast<SeqNodeLAnd*>(this);

            if (1 == pThis->nCountL)
            {
                do
                {
                    SeqNodeL* pNodeL = static_cast<SeqNodeL*>(pThis->arrNodes[0]);
                    pNodeL->Pop(docFrom);
                    pThis->doc = pNodeL->doc;
                }
                while(!(DocumentEOF == pThis->doc || pThis->Filter()));
            }
            else
            {
                do
                {
                    SeqNodeL* pMin  = 0;
                    Document docMin = docFrom;
                    for (SeqNodeL** pp = pThis->BeginL(); pMin != *pp;)
                    {
                        (*pp)->Pop(docMin);
                        if (DocumentEOF == (*pp)->doc)
                        {
                            pThis->doc = DocumentEOF;
                            return;
                        }
                        ffAssert((*pp)->doc <= docMin);
                        ffAssume((*pp)->doc.m.rank <= MaxRank);

                        if ((*pp)->doc < docMin)
                        {
                            docMin = (*pp)->doc;
                            pMin   = *pp;
                        }                          
                        
                        if (++pp == pThis->EndL())
                        {
                            pp = pThis->BeginL();
                        }                        
                    }
                    pThis->doc = docMin;
                    
                }
                while(!pThis->Filter());
                
            }                
            break;
            
        }            
        case sntLOr:
        {
            SeqNodeLOr* pThis = static_cast<SeqNodeLOr*>(this);

            if (DocumentNull == pThis->arrNodes[0]->doc)
            {
                for (SeqNodeL*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
                {
                    ffAssert(DocumentNull == (*pp)->doc);
                    (*pp)->Pop(docFrom);
                    ffAssert(DocumentEOF != (*pp)->doc);
                }           
            }                
             
            uns8_t n1Max = 0;
            for (SeqNodeL*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                if (DocumentEOF != (*pp)->doc && 
                    n1Max < (*pp)->doc + 1)
                {
                    n1Max = (*pp)->doc + 1;
                }
            }
            if (0 == n1Max)
            {
                pThis->doc = DocumentEOF;
            }
            else
            {
                for (SeqNodeL*const* pp = pThis->arrNodes; pp < pThis->End(); ++pp)
                {
                    if (n1Max == (*pp)->doc + 1)
                        (*pp)->Pop(docFrom);
                }                
            
                pThis->doc = n1Max - 1;                
            }

            break;
        }            
        case sntLLeaf:
        {
            SeqNodeLLeaf* pThis = static_cast<SeqNodeLLeaf*>(this);
            if (DocumentMax == docFrom)
            {
                if (!pThis->pField->PsGetNext(pThis->hPostings, pThis->doc))
                    pThis->doc = DocumentEOF;
            }
            else
            {
                if (!pThis->pField->PsGetNextFrom(pThis->hPostings, docFrom, pThis->doc))
                    pThis->doc = DocumentEOF;
            }
            
            break;
        }            
        case sntLFull:
        {   
            SeqNodeLFull* pThis = static_cast<SeqNodeLFull*>(this);
            ffAssert(DocumentMax == docFrom);
            if (pThis->ctx)
            {
                if (!pThis->pDocs->GetNext(pThis->ctx, pThis->doc))
                    pThis->doc = DocumentEOF;                    
            }
            else
            {
                pThis->ctx = pThis->pDocs->GetFirst(pThis->doc);
            }                                              

            break;            
        }
        default:
            ffAssume(false);
    }
}

void SeqNode::Reset() throw()
{
    switch (snt)
    {
        case sntLAnd:
        {
            SeqNodeLAnd* pThis = static_cast<SeqNodeLAnd*>(this);
            pThis->doc = DocumentNull;
            for (SeqNode** pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                (*pp)->Reset();
            }
            break;
        }
        case sntLOr:
        {
            SeqNodeLOr* pThis = static_cast<SeqNodeLOr*>(this);
            pThis->doc = DocumentNull;
            for (SeqNodeL** pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                (*pp)->Reset();
            }
            break;
        }        
        case sntLLeaf:
        {
            SeqNodeLLeaf* pThis = static_cast<SeqNodeLLeaf*>(this);
            pThis->doc = DocumentNull;
            pThis->pField->PsReset(pThis->hPostings);
            break;
        }
        case sntLFull:
        {
            SeqNodeLFull* pThis = static_cast<SeqNodeLFull*>(this);
            pThis->doc = DocumentNull;
            pThis->ctx = 0;
            break;
        }
        case sntDAnd:
        case sntDOr:
        {
            SeqNodeDAndOr* pThis = static_cast<SeqNodeDAndOr*>(this);
            for (SeqNodeD** pp = pThis->arrNodes; pp < pThis->End(); ++pp)
            {
                (*pp)->Reset();
            }
            break;
        }
        case sntDLeaf:
        case sntDLeafNeg:
        {
            SeqNodeDLeaf* pThis = static_cast<SeqNodeDLeaf*>(this);
            pThis->pField->PsReset(pThis->hPostings);
            break;            
        }        
        default:
            ffAssume(false);
    }
}

}//namespace Search
}//namespace FastFish

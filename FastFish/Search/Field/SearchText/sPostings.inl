#include "Search/Documents/sDocuments.h"
#include "Memory/AllocatorSwapper.h"
#include "Tools/Tools.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

ffForceInline
void PsCursor::UpdateRank(const Documents* pDocs) throw()
{
    doc.m.rank  = pDocs->GetRank(doc.m.sid, doc.m.did);
    didLastCR = (0 < doc.m.rank) ? (1 + pDocs->GetDid(doc.m.sid, doc.m.rank - 1)) : 0;
}

inline
size_t Postings::SizeOf(size_t nSegCount) throw()
{
    return sizeof(Postings) + (nSegCount - 1) * (sizeof(PsContext) + sizeof(PsCursor));
}

inline
Postings* Postings::New(size_t nSegCount, AllocatorSwapper& a) throw()
{
    ffAssert(a.ItemSize() == SizeOf(nSegCount));
    PsContext* pCtx = static_cast<PsContext*>(a.Alloc());
    Postings* p      = reinterpret_cast<Postings*>(pCtx + nSegCount - 1);
    
    for (; pCtx <= p->arrCtx; ++pCtx)
    {
        pCtx->nSize = 0;
    }
         
    p->nCardinality = 0;
    p->nCtxCount    = 0;
    p->nCurCount    = InvalidPos;
    
    return p;
}

inline
void Postings::Delete(size_t nSegCount, AllocatorSwapper& a) throw()
{
    ffAssert(a.ItemSize() == SizeOf(nSegCount));
    PsContext* p = reinterpret_cast<PsContext*>(this);
    a.Free(p - nSegCount + 1);
}

inline
void Postings::AddContext(segid_t sid, const PsContext& ctx) throw()
{
    ffAssume(0 != ctx.nSize);
    
    nCardinality += ctx.nSize;

    arrCtx[ - sid] = ctx;
    
    arrCur[nCtxCount++].doc.m.sid = sid;//temporary storage till arrCursors filling, if subset used for .Next
    
    arrCur[sid].doc.m.did = docid_t(-1);//.doc.did & .nPos members used for store PsIsContain context, if subset used for .Find
    arrCur[sid].nPos      = 0;    
}

inline
void Postings::ResetCursors() throw()
{
    PsCursor* pCur = arrCur;
    for (segid_t sid = 0; sid < nCtxCount; ++sid)
    {
        if (0 != arrCtx[ - sid].nSize)
        {
            arrCur[sid].doc.m.did = docid_t(-1);
            arrCur[sid].nPos      = 0;            
            pCur->doc.m.sid = sid;
            ++pCur;        
        }
    }
    nCurCount = InvalidPos;
}

inline
void Postings::InsertCursor(PsCursor curNew) throw()
{
#ifdef ffDebug
    for (size_t n = 0; n < nCurCount; ++n)
    {
        ffAssume(arrCur[n].doc.m.sid != curNew.doc.m.sid);
    }
#endif
    PsCursor* pEnd = arrCur + nCurCount++;
    PsCursor* p    = pEnd - 1;
    for (; arrCur <= p; --p)
    {
        if (CompareRS(p->doc, curNew.doc))
        {
            break;
        }
    }
    ++p;
    memmove(p + 1, p, sizeof(PsCursor) * (pEnd - p));
    *p = curNew;                    
}        


}//namespace Search
}//namespace FastFish


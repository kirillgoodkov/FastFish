#include "Search/Field/SearchText/sFieldSearchText.h"
#include "Search/Documents/sDocuments.h"
#include "Tools/IFStream.h"
#include "Tools/Tools.h"

using namespace std;

namespace FastFish{
namespace Search{

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> ffForceInline
Document FieldSearchText<bSENS, bEMPTYCHECK, tit>::GetNextDocument(Postings* pPs) throw()
{
    PsCursor& cur        = pPs->arrCur[pPs->nCurCount - 1];
    const PsContext& ctx = pPs->arrCtx[ - cur.doc.m.sid];
    Document doc          = cur.doc;
    const PostingsBag& sb = m_vecSeg[cur.doc.m.sid].pb;
    
    if (InvalidPos != cur.nPos)
    {
        sb.Next(ctx, cur);        
        if (cur.doc.m.did < cur.didLastCR)
        {
            if (0 != cur.doc.m.rank)
            {
                cur.UpdateRank(c_pDocs);
            }
            else
            {
                ffAssume(0 == cur.didLastCR);
            }
            --pPs->nCurCount;
            pPs->InsertCursor(cur);
        }
    }
    else
    {
        --pPs->nCurCount;
    }    

    return doc;
}

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
template<bool bUpdateRank> ffForceInline
void FieldSearchText<bSENS, bEMPTYCHECK, tit>::InsertCursorFrom(Postings* pPs, PsCursor cur, Document docFrom) throw()
{
    const PsContext& ctx = pPs->arrCtx[ - cur.doc.m.sid];
    docid_t didSkip = docFrom.m.did;
    if (docFrom.m.sid < cur.doc.m.sid)
    {
        if (0 == docFrom.m.rank)
        {
            return;
        }            
        didSkip = c_pDocs->GetDid(cur.doc.m.sid, docFrom.m.rank - 1);
    }
    else if (cur.doc.m.sid < docFrom.m.sid)
    {
        didSkip = c_pDocs->GetDid(cur.doc.m.sid, docFrom.m.rank);
    }                
    
    const PostingsBag& sb = m_vecSeg[cur.doc.m.sid].pb;
    if (sb.GetCursorFrom(ctx, didSkip, cur))
    {
        if (Static(bUpdateRank) || 0 != cur.doc.m.rank)
        {
            cur.UpdateRank(c_pDocs);
        }
        else
        {
            ffAssume(0 == cur.didLastCR);
        }
        ffAssume(cur.doc.val <= docFrom.val);
        pPs->InsertCursor(cur);
    }
}


//---------------------------------------------------------------------------

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
FieldSearchText<bSENS, bEMPTYCHECK, tit>::FieldSearchText(const TplFieldSearchText& tpl, const Documents* pDocs) throw():
    c_tpl(tpl),
    c_cspn(tpl.pszDelimiters, !tpl.bTextWithControls),
    c_pDocs(pDocs),
    m_vecStrTemp(1, 0)
{
}

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
void FieldSearchText<bSENS, bEMPTYCHECK, tit>::Load(IFStream& ifs) throw()
{
    ffAssertUser(0 == m_pAllocPostings.get() || 0 == m_pAllocPostings->Count(), "active sequence(s) present, free it all before loading new segment");

    uns1_t nVer; ifs >> nVer;
    CheckVersion(nVer, 0);
    
    if (!m_arrCurTemp.IsEmpty())
        m_arrCurTemp.Clear();
    m_arrCurTemp.Alloc(m_vecSeg.size() + 1);
    
    m_vecSeg.push_back(Segment(ifs, c_pDocs));    
    size_t nSizeOfPs = Postings::SizeOf(m_vecSeg.size());
    m_pAllocPostings.reset(new AllocatorSwapper(PC::SearcherPostingsAllocMul*nSizeOfPs, nSizeOfPs, sizeof(size_t)));
}

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
bool FieldSearchText<bSENS, bEMPTYCHECK, tit>::FindPostings(handle_t arr[], size_t& nCount, void* arg) throw()
{
    ffAssume(nCount == MaxWordsInArgument);
    nCount = 0;
    bool bFindAll = true;
    Postings* pPs = Postings::New(m_vecSeg.size(), *m_pAllocPostings);
        
    const char* pszText = reinterpret_cast<const char*>(arg);    
    while (*pszText)
    {
        size_t nLenFull = c_cspn(pszText);        
        if (0 != nLenFull)    
        {
            wordsize_t nLen = wordsize_t(std::min(nLenFull, size_t(c_tpl.nWordLenMax)));            
            const char* pWord = 0;
            if (Static(bSENS))
            {
                nLen  = wordsize_t(UTF8Check(pszText, nLen));
                pWord = pszText;
            }
            else
            {
                if (m_vecStrTemp.capacity() < nLen)
                    m_vecStrTemp.reserve(nLen << 1);
                                 
                nLen  = wordsize_t(UTF8ToLower(pszText, nLen, &m_vecStrTemp.front()));
                pWord = &m_vecStrTemp.front();
            }

            if (UTF8LongerThan(pWord, nLen, c_tpl.nWordLenMin - 1))
            {
                for (segid_t sid = 0; sid < m_vecSeg.size(); ++sid)
                {
                    const Segment& seg = m_vecSeg[sid];
                    size_t nPos = seg.st.FindWord(pWord, nLen);
                    if (InvalidPos != nPos)
                    {
                        pPs->AddContext(sid, seg.pb.GetContext(nPos));
                    }                    
                }
                if (0 == pPs->nCtxCount)
                {
                    bFindAll = false;
                }
                else
                {
                    arr[nCount++] = pPs;
                    pPs = Postings::New(m_vecSeg.size(), *m_pAllocPostings);
                    if (MaxWordsInArgument == nCount)
                    {
                        break;
                    }
                }
            }
            
            pszText += nLenFull;
        }                
        
        if (0 != *pszText)
        {
            ++pszText;
        }            
    }       
    
    pPs->Delete(m_vecSeg.size(), *m_pAllocPostings);    
    return bFindAll;
}

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
bool FieldSearchText<bSENS, bEMPTYCHECK, tit>::PsIsContain(handle_t h, Document doc) throw()
{
    ffAssert(doc.m.sid < m_vecSeg.size());
    Postings* pPs = static_cast<Postings*>(h);
    
    const PsContext& ctx = pPs->arrCtx[ - doc.m.sid];    
    if (ctx.nSize)
    {
        PsCursor& cur = pPs->arrCur[doc.m.sid];
        return m_vecSeg[doc.m.sid].pb.Find(ctx, cur, doc.m.did);
    }
    else
    {
        return false;
    }
}

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
void FieldSearchText<bSENS, bEMPTYCHECK, tit>::PsFree(handle_t h) throw()
{
    static_cast<Postings*>(h)->Delete(m_vecSeg.size(), *m_pAllocPostings);
}

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
bool FieldSearchText<bSENS, bEMPTYCHECK, tit>::PsGetNext(handle_t h, Document& doc) throw()
{
    Postings* pPs = static_cast<Postings*>(h);
    
    if (0 == pPs->nCurCount)
    {
        return false;
    }
    else if (InvalidPos == pPs->nCurCount)
    {//first time initialize
        pPs->nCurCount = 0;
        for (size_t n = 0; n < pPs->nCtxCount; ++n)
        {
            segid_t sid           = pPs->arrCur[n].doc.m.sid;
            const PsContext& ctx  = pPs->arrCtx[ - sid];
            const PostingsBag& sb = m_vecSeg[sid].pb;
            
            PsCursor cur;
            sb.GetCursor(ctx, cur);
            cur.doc.m.sid = sid;
            cur.UpdateRank(c_pDocs);
            pPs->InsertCursor(cur);
        }
    }                 

    doc = GetNextDocument(pPs);
    return true;        
}

template<bool bSENS, bool bEMPTYCHECK, TextIndexType tit> 
bool FieldSearchText<bSENS, bEMPTYCHECK, tit>::PsGetNextFrom(handle_t h, Document docFrom, Document& doc) throw()
{
    Postings* pPs = static_cast<Postings*>(h);
    
    if (0 == pPs->nCurCount)
    {
        return false;
    }
    else if (InvalidPos == pPs->nCurCount)
    {//first time initialize
        pPs->nCurCount = 0;
        for (size_t n = 0; n < pPs->nCtxCount; ++n)
        {
            PsCursor cur;
            cur.doc.m.sid = pPs->arrCur[n].doc.m.sid;
            cur.doc.m.did = docid_t(-1);
            cur.nPos      = 0;
            InsertCursorFrom<true>(pPs, cur, docFrom);
        }
    }                 
    else
    {
        PsCursor* pDst       = m_arrCurTemp.Get();
        const PsCursor* pEnd = pPs->arrCur - 1;
        for (const PsCursor* p = pPs->arrCur + pPs->nCurCount - 1; 
             p != pEnd && docFrom <= p->doc; 
             --p)
        {
            *pDst++ = *p;
        }
        
        size_t nCount = pDst - m_arrCurTemp.Get();
        pPs->nCurCount -= nCount;
        
        pEnd = m_arrCurTemp.Get() + nCount;
        for (const PsCursor* p = m_arrCurTemp.Get(); p < pEnd; ++p)
        {
            InsertCursorFrom<false>(pPs, *p, docFrom);
        }
    }
    if (0 == pPs->nCurCount)
    {
        return false;
    }
    
    doc = GetNextDocument(pPs);
    return true;
}

//---------------------------------------------------------------------------

template class FieldSearchText<false, true, titExact >;
template class FieldSearchText<true , true, titExact >;
template class FieldSearchText<false, true, titPrefix>;  template class FieldSearchText<false, false, titPrefix>;
template class FieldSearchText<true , true, titPrefix>;  template class FieldSearchText<true , false, titPrefix>;
template class FieldSearchText<false, true, titInfix >;  template class FieldSearchText<false, false, titInfix >;
template class FieldSearchText<true , true, titInfix >;  template class FieldSearchText<true , false, titInfix >;

}//namespace Search
}//namespace FastFish

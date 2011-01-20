#include "Search/Field/SearchText/sPostings.h"
#include "Tools/Bits.h"
#include "Common/Consts.h"

namespace FastFish{
namespace Search{

ffForceInline
docid_t PostingsBag::FindCmprBlock(const PsContext& ctx, const PsCursor& cur, docid_t did, const uns1_t*& p) const throw()
{
    size_t nCountRound  = (ctx.cmpr.nBlocks*sizeof(docid_t) + BlockSize - 1) & ~BlockMask;
    const docid_t* pIdx = reinterpret_cast<const docid_t*>(ctx.cmpr.pList - nCountRound);
   
    size_t n = (InvalidPos == cur.nPos) ? (ctx.cmpr.nBlocks - 1) : (cur.nPos >> BlockShift);
    ffAssume(did <= cur.doc.m.did);

    if (m_nJumpFrom < ctx.cmpr.nBlocks * (cur.doc.m.did - did))
    {
        n = std::lower_bound(pIdx + n, pIdx + ctx.cmpr.nBlocks, did, std::greater<docid_t>())
            - pIdx - 1;            
    }
    else
    {
        n = std::find_if(pIdx + n, pIdx + ctx.cmpr.nBlocks, std::bind2nd(std::less_equal<docid_t>(), did))
            - pIdx - 1;            
    }

    p = ctx.cmpr.pList + BlockSize*n;    
    return pIdx[n];
}

//----------------------------------------------------------------------------
template<typename SRC>
size_t ReadLE(const SRC* src, size_t nSize)//LE - little endian
{
    size_t nRes = *src;
#ifdef ff64    
    ffAssume(1 == nSize || 2 == nSize);
    if (1 < nSize)
    {
        nRes |= size_t(*++src) << sizeof(SRC);
    }
#else
    ffAssume(1 == nSize);
#endif   
    return nRes; 
}    

//----------------------------------------------------------------------------
template<typename TYPE> ffForceInline
size_t PostingsBag::BlockIdxA<TYPE>::FindAndRewind(const void* _pIdxA, size_t nPos, const docid_t*& pIdxB) throw()
{
    const BlockIdxA* pIdxA = static_cast<const BlockIdxA*>(_pIdxA);
    pIdxA += nPos / ffBitSizeOf(TYPE);
    nPos &= ffBitSizeOf(TYPE) - 1;    
    TYPE nMask = (1 << nPos) - 1;
    
    size_t nShift = 0, 
           nSize  = 0;
    for (size_t nBit = 0; nBit < FC::PostingsInPlaceShift; ++nBit)
    {
        TYPE val = pIdxA->nBits[nBit];
        nShift  += CountBits(val & nMask) << nBit;
        nSize   |= ((val >> nPos) & 1) << nBit;
    }    
    
    pIdxB += pIdxA->nBase + nShift;                
    return nSize;
}

//----------------------------------------------------------------------------
ffForceInline
PsContext PostingsBag::GetContext(size_t nPos) const throw()
{
    PsContext ctx;

    const docid_t* pIdxB  = m_pIdxB;
    size_t nSizeB = 
        #ifdef ff64
            m_bLongIdxA ? 
                BlockIdxA<uns8_t>::FindAndRewind(m_pIdxA, nPos, pIdxB):
                BlockIdxA<uns4_t>::FindAndRewind(m_pIdxA, nPos, pIdxB);
        #else
                BlockIdxA<uns4_t>::FindAndRewind(m_pIdxA, nPos, pIdxB);
        #endif
    
    
    if (nSizeB && (*pIdxB & FC::PostingsExternalFlag))
    {
        ctx.nSize = (*pIdxB++) & ~FC::PostingsExternalFlag;
        ctx.did0  = *pIdxB++;
        if (m_nBmpStartFrom <= ctx.nSize)               
        {//bitmap
            ctx.bmp.did1 = *pIdxB++;
            ctx.bmp.p    = m_pBitmaps + 
                           ReadLE(pIdxB, nSizeB - 3) * m_nBmpSize;                                 
        }
        else if (FC::PostingsFixedListMaxSize < ctx.nSize)   
        {//compressed list
            ctx.cmpr.nBlocks = *pIdxB++;
            size_t nClass    = ffBitSizeOf(ctx.nSize) - CountLeftNulls(ctx.nSize) - FC::PostingsCmprListMinShift;
            ctx.cmpr.pList   = m_arrCmpr[nClass] + 
                               ReadLE(pIdxB, nSizeB - 3) * BlockSize;
        }
        else if (FC::PostingsInPlace < ctx.nSize)                
        {//sorted list
            ctx.lst.p = m_arrFixed[ctx.nSize - FC::PostingsInPlace - 1] + 
                        ReadLE(pIdxB, nSizeB - 2) * ctx.nSize;
        }
    }
    else
    {//inplace
        ctx.nSize = nSizeB;
        ctx.did0  = *pIdxB;
        ctx.lst.p = pIdxB;
    }        
    return ctx;    
}

ffForceInline
bool PostingsBag::Find(const PsContext& ctx, PsCursor& cur, docid_t did) const throw()
{   
    if (m_nBmpStartFrom <= ctx.nSize)
    {//bitmap
        return GetBit(ctx.bmp.p, m_nDocumentsCount - did - 1);
    }
    else if (FC::PostingsFixedListMaxSize < ctx.nSize)
    {//compressed list
        if (cur.doc.m.did <= did)
        {
            return cur.doc.m.did == did;
        }            

        const uns1_t* p;
        cur.doc.m.did = FindCmprBlock(ctx, cur, did, p);
        const uns1_t* pEnd = p + BlockSize; 
        
        for (;;)
        {
            cur.doc.m.did -= VbeRead(p);
            if (cur.doc.m.did == did)
            {
                cur.nPos = p - ctx.cmpr.pList;
                return true;           
                
            }                
            if (cur.doc.m.did < did || pEnd == p)                    
            {
                cur.nPos = p - ctx.cmpr.pList;
                return false;           
            }                
        } 
    }
    else
    {//sorted list || inplace
        return std::binary_search(ctx.lst.p, ctx.lst.p + ctx.nSize, did, std::greater<docid_t>());
    }
}

ffForceInline
void PostingsBag::Next(const PsContext& ctx, PsCursor& cur) const throw()
{
    ffAssume(InvalidPos != cur.nPos);
    
    if (m_nBmpStartFrom <= ctx.nSize) 
    {//bitmap
        cur.doc.m.did = m_nDocumentsCount - docid_t(cur.nPos) - 1;
        cur.nPos      = FindBit(ctx.bmp.p, 1 + cur.nPos);

        if (m_nDocumentsCount == cur.nPos)
        {
            cur.nPos = InvalidPos;
        }
    }
    else if (FC::PostingsFixedListMaxSize < ctx.nSize) 
    {//compressed list
        const uns1_t* p = ctx.cmpr.pList + cur.nPos;

        uns4_t nDiff = VbeRead(p);
        for (; 0 == *p; ++p)
        {}                
        
        ffAssume(0 < nDiff && nDiff <= cur.doc.m.did);
        
        cur.doc.m.did -= nDiff;            
        cur.nPos       = p - ctx.cmpr.pList;
        
        if ((cur.nPos >> BlockShift) >= ctx.cmpr.nBlocks)
        {
            cur.nPos = InvalidPos;
        }    
    }
    else
    {//sorted list || inplace
        cur.doc.m.did = ctx.lst.p[cur.nPos++];
        
        if (ctx.nSize == cur.nPos)
            cur.nPos = InvalidPos;
    }
}

ffForceInline
void PostingsBag::GetCursor(const PsContext& ctx, PsCursor& cur) const throw()
{
    cur.doc.m.did = ctx.did0;
    if (m_nBmpStartFrom <= ctx.nSize) 
    {//bitmap    
        cur.nPos = m_nDocumentsCount - ctx.bmp.did1 - 1;
    }
    else if (FC::PostingsFixedListMaxSize < ctx.nSize) 
    {//compressed list
        cur.nPos = VbeSizeOf(m_nDocumentsCount - cur.doc.m.did);
    }
    else
    {//sorted list || inplace
        cur.nPos = (1 == ctx.nSize) ? InvalidPos : 1;
    }
}

ffForceInline
bool PostingsBag::GetCursorFrom(const PsContext& ctx, docid_t didFrom, PsCursor& cur) const throw()
{   
    if (m_nDocumentsCount == didFrom)
    {
        GetCursor(ctx, cur);
        return true;
    }
    
    ffAssume(didFrom < m_nDocumentsCount);
    if (m_nBmpStartFrom <= ctx.nSize) 
    {//bitmap
        cur.nPos = FindBit(ctx.bmp.p, m_nDocumentsCount - didFrom - 1);
        if (m_nDocumentsCount == cur.nPos)
        {
            cur.nPos = InvalidPos;
            return false;
        }                        
    }
    else if (FC::PostingsFixedListMaxSize < ctx.nSize) 
    {//compressed list
        const uns1_t* p;
        docid_t did = FindCmprBlock(ctx, cur, didFrom, p);
        const uns1_t* pEnd = p + BlockSize; 

        for (;;)
        {
            const uns1_t* pPrev = p;
            cur.doc.m.did       = did;                        
            did                -= VbeRead(p);                    
            
            if (did <= didFrom)
            {
                for (; 0 == *pPrev; ++pPrev)
                {}                
                ffAssume(ctx.cmpr.pList <= pPrev);
                cur.nPos = pPrev - ctx.cmpr.pList;
                break;
            }                
            if (pEnd <= p)                    
            {
                cur.nPos = InvalidPos;
                return false;
            }                
        } 
    }
    else
    {//sorted list || inplace
        cur.nPos = std::lower_bound(ctx.lst.p, ctx.lst.p + ctx.nSize, didFrom, std::greater<docid_t>()) - ctx.lst.p;
        if (ctx.nSize == cur.nPos)
        {
            cur.nPos = InvalidPos;
            return false;
        }                   
    }
    
    Next(ctx, cur);
    return true;       
}

}//namespace Search
}//namespace FastFish

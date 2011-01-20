#include "Build/Field/SearchText/bPostingsBag.h"
#include "Build/Field/SearchText/Visitors/CreateBitmap.h"
#include "Build/Field/SearchText/Visitors/CreateCmprList.h"
#include "Build/Field/SearchText/Visitors/ReorderBFS.h"
#include "Build/Documents/bDocumentsNoRanks.h"
#include "Build/Documents/bDocumentsRanked.h"

using namespace std;

namespace FastFish{
namespace Build{

template<typename DST>
void PushBackLE(DST& dst, size_t nVal) throw()//LE - little endian
{
    dst.PushBack(typename DST::ValueType(nVal));
#ifdef ff64    
    if (DST::ValueType(-1) < nVal)
    {
        dst.PushBack(typename DST::ValueType(nVal >> ffBitSizeOf(typename DST::ValueType)));    
    }
#endif    
}

template<typename DOCUMENTS, typename CONT>
struct TransformSet
{
    const DOCUMENTS*    pDocs;
    CONT*               pCont; 
    
    void operator()(docid_t did) const throw()
    {
        pCont->PushBack(pDocs->TransformForw(did));
    }
};                     

template<typename DOCUMENTS>
struct TransformStore
{
    const DOCUMENTS*    pDocs;
    OFStream*           pFile; 
    docid_t             did0;
    
    void operator()(docid_t did) ffThrowAll
    {
        did = pDocs->TransformForw(did);
        *pFile << did;
        if (docid_t(-1) == did0)
        {
            did0 = did;
        }
    }
};

//---------------------------------------------------------------------------

template<typename DOCUMENTS, bool bCTRL, bool bSENS, TextIndexType tit> 
PostingsBag<DOCUMENTS, bCTRL, bSENS, tit>::PostingsBag(const BFS& bfs, const DOCUMENTS* pDocs) throw():
    c_bfs(bfs),
    c_pDocs(pDocs),
    c_nBmpStartFrom(max(pDocs->Count()/FC::PostingsBmpStartFromDiv, docid_t(FC::PostingsFixedListMaxSize + 1)))
{
    ffAssumeStatic(5 <= FC::PostingsInPlace);
    
    typedef typename Node<bCTRL, bSENS, tit>::SetType SetType;    
    
    size_t arrCmprMaxPos[ffCountOf(m_arrDeqCmpr)] = {0};
    
    for (size_t n = 0; n < c_bfs.c_nCount; ++n)
    {
        const SetType& setPostings = c_bfs.At(n).Postings();

        m_deqIdxA.PushBack(m_deqIdxB.Count());
        docid_t nCount = setPostings.Count();

        if (nCount <= FC::PostingsInPlace)        
        {//inplace
            TransformSet<DOCUMENTS, DeqIdxB> ts = {pDocs, &m_deqIdxB};
            setPostings.Enum(ts);            
        }
        else
        {
            IndexRef ir = {n, m_deqIdxB.Count()};
            m_deqIdxB.PushBack(nCount | FC::PostingsExternalFlag);    //nCount            
            m_deqIdxB.PushBack(0);                                  //doc 0
            if (c_nBmpStartFrom <= nCount)
            {//bmp
                m_deqIdxB.PushBack(0);                              //doc 1
                PushBackLE(m_deqIdxB, m_deqBmp.Count());            //nPos
                m_deqBmp.PushBack(ir);
            }
            else if (FC::PostingsFixedListMaxSize < nCount)
            {//cmpr
                size_t nClass = ffBitSizeOf(nCount) - CountLeftNulls(nCount) - FC::PostingsCmprListMinShift;
                arrCmprMaxPos[nClass] += nCount * CreateCmprList<DOCUMENTS>::MaxItemSize;
                m_deqIdxB.PushBack(0);                              //CountBlocks
                m_deqIdxB.PushBack(0);                              //nPos
                if (docid_t(-1) < (arrCmprMaxPos[nClass] >> FC::PostingsCmprListBlockShift))
                    m_deqIdxB.PushBack(0);                          //nPos*                
                m_arrDeqCmpr[nClass].PushBack(ir);
            }
            else
            {//fix
                size_t nIdx = nCount - FC::PostingsInPlace - 1;
                ffAssume(nIdx < ffCountOf(m_arrDeqFixed));
                DeqIdxIr& deq = m_arrDeqFixed[nIdx];
                PushBackLE(m_deqIdxB, deq.Count());                 //nPos*               
                deq.PushBack(ir);
            }            
        }
    }    
    m_deqIdxA.PushBack(m_deqIdxB.Count());//fake item for estimate last diff
}

template<typename DOCUMENTS, bool bCTRL, bool bSENS, TextIndexType tit> 
template<typename IDXA_TYPE> ffForceInline
void PostingsBag<DOCUMENTS, bCTRL, bSENS, tit>::StoreIdxA(OFStream& file) const ffThrowAll
{
    struct BlockIdxA
    {
        IDXA_TYPE nBase;
        IDXA_TYPE nBits[FC::PostingsInPlaceShift];
    };              

    file.BagBegin(sizeof(BlockIdxA));
    
    for (size_t nBase = 0; nBase < m_deqIdxA.Count() - 1; nBase += ffBitSizeOf(IDXA_TYPE))
    {
        BlockIdxA block = {IDXA_TYPE(m_deqIdxA[nBase])};
        fill(block.nBits, block.nBits + ffCountOf(block.nBits), 0);
        
        size_t nSubs = min(nBase + ffBitSizeOf(IDXA_TYPE), m_deqIdxA.Count() - 1);
        for (size_t n = nBase; n < nSubs; ++n)
        {
            size_t nCount = m_deqIdxA[n + 1] - m_deqIdxA[n];
            for (size_t nBit = 0; nBit < FC::PostingsInPlaceShift; ++nBit, nCount >>= 1)
            {
                block.nBits[nBit] |= IDXA_TYPE(nCount & 1) << (n - nBase);
            }
        }
        file.Write(&block, sizeof(block));
    }
    
    file.BagCommit();                
    ffLog(lsInfoFileSummary, "\tidx.a:    "); file.DumpSize();
}


template<typename DOCUMENTS, bool bCTRL, bool bSENS, TextIndexType tit> 
void PostingsBag<DOCUMENTS, bCTRL, bSENS, tit>::Store(OFStream& file) ffThrowAll
{
    typedef typename Node<bCTRL, bSENS, tit>::SetType SetType;    
    
    file << uns1_t(0)//version
         << bool(MaxUns4 < m_deqIdxB.Count())
         << c_pDocs->Count()
         << c_nBmpStartFrom;          

    {//fixed size lists
        for (size_t nClass = 0; nClass < ffCountOf(m_arrDeqFixed); ++nClass)
        {
            DeqIdxIr& deq = m_arrDeqFixed[nClass];
            
            file.BagBegin(sizeof(docid_t));            
            for (size_t n = 0; n < deq.Count(); ++n)
            {
                const IndexRef& ir = deq[n];
                const SetType& setPostings = c_bfs.At(ir.nNode).Postings();
                TransformStore<DOCUMENTS> ts = {c_pDocs, &file, docid_t(-1)};
                setPostings.Enum(ts);
                ffAssume(docid_t(-1) != ts.did0);
                m_deqIdxB[ir.nIdxB + 1] = ts.did0;
            }                                            
            file.BagCommit();                
            deq.Clear();
        }
        ffLog(lsInfoFileSummary, "\tpst.fix:  "); file.DumpSize();
    }
         
    {//compressed lists

        for (size_t nClass = 0; nClass < ffCountOf(m_arrDeqCmpr); ++nClass)
        {
            DeqIdxIr& deq = m_arrDeqCmpr[nClass];
            
            file.BagBegin(1 << FC::PostingsCmprListBlockShift);            
            filepos_t nBasePos = file.GetBagPos();
            for (size_t n = 0; n < deq.Count(); ++n)
            {
                const IndexRef& ir = deq[n];
                const SetType& setPostings = c_bfs.At(ir.nNode).Postings();
                CreateCmprList<DOCUMENTS> ccl(setPostings, c_pDocs);
                filepos_t nPos          = ccl.Store(file, nBasePos);
                m_deqIdxB[ir.nIdxB + 1] = ccl.Document0();
                m_deqIdxB[ir.nIdxB + 2] = ccl.CountBlocks();
                m_deqIdxB[ir.nIdxB + 3] = docid_t(nPos);
                #ifdef ff64
                    if (docid_t(-1) < nPos)
                        m_deqIdxB[ir.nIdxB + 4] = docid_t(nPos >> ffBitSizeOf(docid_t));
                #endif                
            }            
            file.BagCommit();
            deq.Clear();
        }            
        ffLog(lsInfoFileSummary, "\tpst.cmpr: "); file.DumpSize();            
    }
    
    {//bitmaps
        file.BagBegin(sizeof(bmpword_t));
        for (size_t n = 0; n < m_deqBmp.Count(); ++n)
        {
            const IndexRef& ir = m_deqBmp[n];
            const SetType& setPostings = c_bfs.At(ir.nNode).Postings();
            CreateBitmap<DOCUMENTS> visBmp(c_pDocs);
            setPostings.Enum(visBmp);
            file << visBmp;                
            m_deqIdxB[ir.nIdxB + 1] = visBmp.arrDocuments[0];
            m_deqIdxB[ir.nIdxB + 2] = visBmp.arrDocuments[1];
        }
        file.BagCommit();
        m_deqBmp.Clear();
        ffLog(lsInfoFileSummary, "\tpst.bmp:  "); file.DumpSize();
    }
    
    file.BagBegin(sizeof(docid_t));
    file << m_deqIdxB;
    file.BagCommit();     
    ffLog(lsInfoFileSummary, "\tidx.b:    "); file.DumpSize();

    if (uns4_t(-1) < m_deqIdxB.Count())
    {
        StoreIdxA<uns8_t>(file);
    }
    else
    {
        StoreIdxA<uns4_t>(file);
    }
}

template class PostingsBag<DocumentsNoRanks, false, false, titExact >;  template class PostingsBag<DocumentsRanked, false, false, titExact >;
template class PostingsBag<DocumentsNoRanks, false, true , titExact >;  template class PostingsBag<DocumentsRanked, false, true , titExact >;
template class PostingsBag<DocumentsNoRanks, true , false, titExact >;  template class PostingsBag<DocumentsRanked, true , false, titExact >;
template class PostingsBag<DocumentsNoRanks, true , true , titExact >;  template class PostingsBag<DocumentsRanked, true , true , titExact >;
template class PostingsBag<DocumentsNoRanks, false, false, titPrefix>;  template class PostingsBag<DocumentsRanked, false, false, titPrefix>;
template class PostingsBag<DocumentsNoRanks, false, true , titPrefix>;  template class PostingsBag<DocumentsRanked, false, true , titPrefix>;
template class PostingsBag<DocumentsNoRanks, true , false, titPrefix>;  template class PostingsBag<DocumentsRanked, true , false, titPrefix>;
template class PostingsBag<DocumentsNoRanks, true , true , titPrefix>;  template class PostingsBag<DocumentsRanked, true , true , titPrefix>;
template class PostingsBag<DocumentsNoRanks, false, false, titInfix >;  template class PostingsBag<DocumentsRanked, false, false, titInfix >;
template class PostingsBag<DocumentsNoRanks, false, true , titInfix >;  template class PostingsBag<DocumentsRanked, false, true , titInfix >;
template class PostingsBag<DocumentsNoRanks, true , false, titInfix >;  template class PostingsBag<DocumentsRanked, true , false, titInfix >;
template class PostingsBag<DocumentsNoRanks, true , true , titInfix >;  template class PostingsBag<DocumentsRanked, true , true , titInfix >;

}//namespace Build
}//namespace FastFish

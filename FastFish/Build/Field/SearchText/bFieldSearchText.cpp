#include "Build/Field/SearchText/bFieldSearchText.h"
#include "Build/Field/SearchText/Visitors/GatherStatistic.h"
#include "Build/Field/SearchText/Visitors/ReorderBFS.h"
#include "Build/Field/SearchText/bPostingsBag.h"
#include "Build/Documents/bDocumentsNoRanks.h"
#include "Build/Documents/bDocumentsRanked.h"
#include "Tools/OFStream.h"
#include "Tools/Bits.h"
#include "Common/Templates.h"

using namespace std;

namespace FastFish{
namespace Build{

template<bool bCTRL, bool bSENS, TextIndexType tit>
FieldSearchText<bCTRL, bSENS, tit>::FieldSearchText(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw():
    c_tplIndex(tplIndex),
    c_tpl(tpl),
    c_pDocs(pDocs),
    m_a(a),
    m_arrRanks(tplIndex.nRanksCount),
    m_nNodesCount(0)
{
}

template<bool bCTRL, bool bSENS, TextIndexType tit>
FieldSearchText<bCTRL, bSENS, tit>::~FieldSearchText() throw() 
{
};

template<bool bCTRL, bool bSENS, TextIndexType tit>
void FieldSearchText<bCTRL, bSENS, tit>::Finalize() ffThrowNone
{
    ffAssertUser(m_treeMerged.val.IsEmpty(), "Finalize called twice");
    
    m_treeMerged.val = m_arrRanks[0].val;
    for (size_t n = 1; n < c_tplIndex.nRanksCount; ++n)
    {
        size_t nOldCount = m_a.aMap.aItem.Count();
        m_treeMerged.val.Merge(m_arrRanks[n].val, m_a.aMap, m_a);
        m_nNodesCount   -= nOldCount - m_a.aMap.aItem.Count();
    }    

    if (ffIsLogged(lsInfoTopPrefix))
    {
        docid_t n = docid_t(c_pDocs->Count()*TopPrefixMul);
        ffLog(lsInfoTopPrefix, "\ntop prefixes (<%u items):\n", n);
        GatherStatistic visStat(n);
        m_treeMerged.val.Enum(visStat);    
    }
    
    ffAssert(0 == m_pBFS.get());
    m_pBFS.reset(new BFS(m_nNodesCount));
    m_pBFS->Reorder(m_treeMerged.val);
}

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
bool FieldSearchText<bCTRL, bSENS, tit>::TryStore(OFStream& file, const DeqSpans& deqSpans, size_t nSizeOfNode, size_t nSizeOfSuffix) const ffThrowAll
{
    DeqSuffixs deqSuffixPtrs;

    size_t     nPos       = 0;
    filepos_t  nSuffixPos = 0;        
    const filepos_t nSuffixPosLimit = filepos_t(-1) >> BitsInByte*(sizeof(filepos_t) - nSizeOfSuffix);

    //nodes ------------------------------------------------------------------
    file.BagBegin();        
    for (size_t nSpan = 0; nSpan < deqSpans.Count(); ++nSpan)
    {
        size_t nCount = deqSpans[nSpan];
        
        m_pBFS->StoreCharsFrom(file, nPos, nCount);
        
        for (size_t n = 0; n < nCount; ++n)
        {
            const NodeType& node = m_pBFS->At(n + nPos);
            
            size_t nSizeOfInPlace = node.Childs().IsEmpty() ? (nSizeOfSuffix + nSizeOfNode) : nSizeOfSuffix;
            bool bInPlace         = node.SuffixLen() <= nSizeOfInPlace;
            
            uns1_t nChildCount = ((FC::TreeBmpStartFrom <= node.Childs().Count()) ? FC::TreeBmpStartFrom : node.Childs().Count()) |
                                 (bInPlace ? FC::TreeSfxInPlaceFlag : 0) |
                                 (node.Postings().IsEmpty() ? FC::TreeEmptySetFlag : 0);   
            file << nChildCount;
            
            if (bInPlace)            
            {
                file.Write(node.Suffix(), node.SuffixLen());
                file.WriteNulls(nSizeOfInPlace - node.SuffixLen());
            }
            else
            {
                if ((nSuffixPosLimit - nSuffixPos) < filepos_t(node.SuffixLen() + 1))
                {
                    file.BagDiscard();
                    return false;
                }
                
                Suffix sfx = {node.Suffix(), node.SuffixLen()};
                deqSuffixPtrs.PushBack(sfx);
                file.WriteLowPart(nSuffixPos, nSizeOfSuffix);
                file.WriteNulls(nSizeOfInPlace - nSizeOfSuffix);
                
                nSuffixPos += node.SuffixLen() + 1;                
            }
            
            if (!node.Childs().IsEmpty())                                                 
            {
                const NodeType* pChild = node.Childs().GetFirst();
                file.WriteLowPart(size_t(pChild), nSizeOfNode);
            }                
        }              
        if (FC::TreeBmpStartFrom < nCount)          
        {
            file.WriteNulls(nCount - FC::TreeBmpStartFrom);
        }
        nPos += nCount;
    }
    ffAssume(nPos == m_nNodesCount);
    
    file.BagCommit();
    ffLog(lsInfoFileSummary, "\ttree:     "); file.DumpSize();
    file << uns1_t(nSizeOfSuffix);
    
    //suffixes ---------------------------------------------------------------
    file.BagBegin();
    for (size_t n = 0; n < deqSuffixPtrs.Count(); ++n)
    {
        const Suffix& sfx = deqSuffixPtrs[n];
        file.Write(sfx.p, sfx.n);
        file << uns1_t(0);
    }
    
    ffLog(lsInfoFileSummary, "\tsfx:      "); file.DumpSize();
    file.BagCommit();
    
    return true;
}

template<bool bCTRL, bool bSENS, TextIndexType tit>
void FieldSearchText<bCTRL, bSENS, tit>::Store(OFStream& file) const ffThrowAll
{
    ffAssert(m_pBFS.get());
    ffAssumeStatic(FC::TreeBmpStartFrom < FC::TreeSfxInPlaceFlag);
    ffAssumeStatic(SizeUns1/BitsInByte <= FC::TreeBmpStartFrom);

    file << uns1_t(0)//version
         << uns8_t(m_nNodesCount)
         << uns1_t(m_treeMerged.val.Count());
    
    {         
        DeqSpans deqSpans;
        deqSpans.PushBack(m_treeMerged.val.Count());
        for (size_t n = 0; n < m_pBFS->c_nCount; ++n)
        {
            size_t nCount = m_pBFS->At(n).Childs().Count();
            if (nCount)
                deqSpans.PushBack(DeqSpans::ValueType(nCount));
        }         
        
        size_t nSizeOfNode = Value2SizeOf(m_nNodesCount);
        
        for (size_t nSizeOfSuffix = nSizeOfNode;
             !TryStore(file, deqSpans, nSizeOfNode, nSizeOfSuffix); 
             ++nSizeOfSuffix)
        {        
            ffAssume(nSizeOfSuffix <= sizeof(size_t));
        }
    }        
    
    //sets-------------------------------------------------------------------
    if (1 == c_tplIndex.nRanksCount)
    {
        PostingsBag<DocumentsNoRanks, bCTRL, bSENS, tit> pb(*m_pBFS, static_cast<const DocumentsNoRanks*>(c_pDocs));
        pb.Store(file);
    }
    else
    {
        PostingsBag<DocumentsRanked, bCTRL, bSENS, tit> pb(*m_pBFS, static_cast<const DocumentsRanked*>(c_pDocs));
        pb.Store(file);                       
    }
}


//---------------------------------------------------------------------------
template class FieldSearchText<false, false, titExact>;
template class FieldSearchText<false, true , titExact>;
template class FieldSearchText<true,  false, titExact>;
template class FieldSearchText<true,  true , titExact>;
template class FieldSearchText<false, false, titPrefix>;
template class FieldSearchText<false, true , titPrefix>;
template class FieldSearchText<true,  false, titPrefix>;
template class FieldSearchText<true,  true , titPrefix>;
template class FieldSearchText<false, false, titInfix>;
template class FieldSearchText<false, true , titInfix>;
template class FieldSearchText<true,  false, titInfix>;
template class FieldSearchText<true,  true , titInfix>;

}//namespace Build
}//namespace FastFish

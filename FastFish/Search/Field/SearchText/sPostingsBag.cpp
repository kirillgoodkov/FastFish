#include "Search/Field/SearchText/sPostingsBag.h"
#include "Search/Documents/sDocuments.h"
#include "Search/Layers.h"
#include "Tools/IFStream.h"
#include "Tools/Tools.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Search{

PostingsBag::PostingsBag(IFStream& ifs, const Documents* pDocs) throw():
    m_pDocs(pDocs)
{
    uns1_t nVer; ifs >> nVer;
    CheckVersion(nVer, 0);
    
    ifs >> m_bLongIdxA
        >> m_nDocumentsCount
        >> m_nBmpStartFrom;
        
    m_nBmpSize  = (m_nDocumentsCount/ffBitSizeOf(bmpword_t) + 1);
    m_nJumpFrom = m_nDocumentsCount * PC::SearcherLongJumpBlocks;
    
    for (size_t n = 0; n < ffCountOf(m_arrFixed); ++n)
    {
        m_arrFixed[n] = ifs.GetBag<docid_t>(liSearchPst, sizeof(docid_t));
    }    
    for (size_t n = 0; n < ffCountOf(m_arrCmpr); ++n)
    {
        m_arrCmpr[n] = ifs.GetBag<uns1_t>(liSearchPst, BlockSize);
    }
            
    m_pBitmaps = ifs.GetBag<bmpword_t>(liSearchPst, sizeof(bmpword_t));
    m_pIdxB    = ifs.GetBag<docid_t>(liSearchIdxB, sizeof(docid_t));
    m_pIdxA    = ifs.GetBagRaw(liSearchIdxA, m_bLongIdxA ? sizeof(BlockIdxA<uns8_t>) : sizeof(BlockIdxA<uns4_t>));
}

}//namespace Search
}//namespace FastFish

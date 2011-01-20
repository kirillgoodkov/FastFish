#pragma once
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class PostingsBag
{
public:
    PostingsBag(IFStream& ifs, const Documents* pDocs)                              throw();
    
    PsContext GetContext(size_t nPos)                                         const throw();
    void      GetCursor(const PsContext& ctx, PsCursor& cur)                  const throw();
    bool      GetCursorFrom(const PsContext& ctx, docid_t did, PsCursor& cur) const throw();
    bool      Find(const PsContext& ctx, PsCursor& cur, docid_t did)          const throw();
    void      Next(const PsContext& ctx, PsCursor& cur)                       const throw();
    
private:
    docid_t FindCmprBlock(const PsContext& ctx, const PsCursor& cur, docid_t did, const uns1_t*& p) const throw();

    static const size_t BlockShift = FC::PostingsCmprListBlockShift;
    static const size_t BlockSize  = 1 << BlockShift;
    static const size_t BlockMask  = BlockSize - 1;

    const Documents*    m_pDocs;
                        
    bool                m_bLongIdxA;                    
    docid_t             m_nDocumentsCount;
    docid_t             m_nBmpStartFrom;
    
    docid_t             m_nBmpSize;     
    size_t              m_nJumpFrom;
    
    const docid_t*      m_arrFixed[FC::PostingsFixedListMaxSize - FC::PostingsInPlace];
    const uns1_t*       m_arrCmpr[FC::PostingsCmprListMaxShift - FC::PostingsCmprListMinShift];
    const bmpword_t*    m_pBitmaps;       
                        
    const docid_t*      m_pIdxB;
    const void*         m_pIdxA;    
    
    template<typename TYPE>
    struct BlockIdxA
    {
        TYPE nBase;
        TYPE nBits[FC::PostingsInPlaceShift];
        
        static size_t FindAndRewind(const void* pIdxA, size_t nPos, const docid_t*& pIdxB) throw();
    };              
    
};
   
}//namespace Search
}//namespace FastFish

#include "Search/Field/SearchText/sPostingsBag.inl"


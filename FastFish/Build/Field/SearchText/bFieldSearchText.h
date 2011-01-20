#pragma once
#include "Build/Field/bField.h"
#include "Build/Field/SearchText/bNode.h"
#include "Memory/ArrayPtr.h"
#include "Tools/Strings.h"              
#include "Common/Common.h"

namespace FastFish{
namespace Build{

template<bool bCTRL, bool bSENS, TextIndexType tit>
class FieldSearchText:
    public Field
{
    FieldSearchText(const FieldSearchText&);
    FieldSearchText& operator = (const FieldSearchText&);
public:
    FieldSearchText(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDoc, TextAllocator& a) throw();
    virtual ~FieldSearchText()                                                                                        throw();

    virtual void Finalize()                  ffThrowNone;
    virtual void Store(OFStream& file) const ffThrowAll;
    
    void CommitWord(rank_t rank, const char* pWord, wordsize_t nLen) throw();
    
protected:
    const TplIndex&             c_tplIndex;
    const TplFieldSearchText&   c_tpl;
    const Documents*const       c_pDocs;

    TextAllocator&      m_a;    
    
private:
    struct Suffix
    {
        const char* p;
        wordsize_t  n;
    };

    typedef Node<bCTRL, bSENS, tit>                     NodeType;
    typedef CharMapSmall<NodeType, bCTRL, bSENS>        MapType;
    typedef CharMapSmallPd<NodeType, bCTRL, bSENS>      MapTypePd;
    typedef ReorderBFS<bCTRL, bSENS, tit>               BFS;
    typedef std::auto_ptr<BFS>                          BFSPtr;
    typedef Deque<Suffix, PC::FieldTextDeqSuffixShift>  DeqSuffixs;
    typedef Deque<uns1_t, PC::FieldTextDeqSpansShift>   DeqSpans;

    bool TryStore(OFStream& file, const DeqSpans& deqSpans, size_t nSizeOfNode, size_t nSizeOfSuffix) const ffThrowAll;
   
    MapTypePd               m_treeMerged;
    ArrayPtr<MapTypePd>     m_arrRanks;    
    size_t                  m_nNodesCount;    
    BFSPtr                  m_pBFS;
};

}//namespace Build
}//namespace FastFish
#include "Build/Field/SearchText/bFieldSearchText.inl"

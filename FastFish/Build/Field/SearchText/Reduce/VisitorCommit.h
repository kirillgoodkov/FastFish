#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Build{
namespace Reduce{

template<bool bCTRL, bool bSENS, TextIndexType tit>
class VisitorCommit
{
    VisitorCommit(const VisitorCommit&);
    VisitorCommit& operator = (const VisitorCommit&);
public:
    typedef FieldSearchText<bCTRL, bSENS, tit>  Field;
    typedef Node<bCTRL, bSENS>                  NodeT;

    VisitorCommit(char* pBuffer, Field* pFld, rank_t rank) throw();

    void operator()(char ch, const NodeT* pNode) throw();

private:
    const char*const    m_pStrBegin;

    char*               m_pStr;
    Field*              m_pField;
    rank_t              m_rank;
};
    


}//namespace Reduce
}//namespace Build
}//namespace FastFish
#include "VisitorCommit.inl"

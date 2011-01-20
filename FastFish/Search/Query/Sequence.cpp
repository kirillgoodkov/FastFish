#include "Search/Query/Sequence.h"
#include "Search/Query/SeqNodes.h"


namespace FastFish{
namespace Search{

Sequence::Sequence(size_t nAproxLen, SeqNodeL* pRoot) throw():
    c_nApproximateLength(nAproxLen),
    c_pRoot(pRoot),
    m_nPos(InitPos)
{
}

Sequence::~Sequence() throw()
{
    c_pRoot->FreePostings();
}

void Sequence::Next() throw()
{
    c_pRoot->Pop();
    if (DocumentEOF == c_pRoot->doc)
    {
        m_nPos = EndPos;
    }
    else
    {
        ++m_nPos;
    }  
}

void Sequence::First() throw()
{
    if (InitPos != m_nPos)
    {
        m_nPos = InitPos;
        c_pRoot->Reset();
    }
    Next();
}

size_t Sequence::GetApproximateLength() const throw()
{
    return c_nApproximateLength;
}

}//namespace Search
}//namespace FastFish

#include "Search/Query/SeqNodes.h"
namespace FastFish{
namespace Search{

inline
size_t Sequence::SizeOf(size_t nDataSize) throw() 
{
    ffAssume(0 == (nDataSize & 7));    
    return sizeof(Sequence) + nDataSize - 1;
}

inline
size_t Sequence::GetPosition() const throw()
{
    ffAssumeUser(InitPos != m_nPos, "call First before using GetPosition");
    ffAssumeUser(EndPos != m_nPos, "sequence is finished");
    return m_nPos;
}

inline
bool Sequence::IsFinished() const throw()
{
    return EndPos == m_nPos;
}

inline
Sequence::operator Document() const throw()
{
    ffAssumeUser(InitPos != m_nPos, "call First before using Record()");
    ffAssumeUser(EndPos != m_nPos, "end of sequence");
    return c_pRoot->doc;
}




}//namespace Search
}//namespace FastFish

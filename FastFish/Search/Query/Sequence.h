#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class Sequence
{
    friend class Query;
    friend class Optimizer;
    friend class std::auto_ptr<Sequence>;

    Sequence(const Sequence&) throw();
    Sequence& operator = (const Sequence&) throw();

    Sequence(size_t nAproxLen, SeqNodeL* pRoot) throw();
    ~Sequence() throw();    
public:                                   
    void First()                          throw();
    void Next()                           throw();
    
    operator Document()             const throw();
    bool     IsFinished()           const throw();
    size_t   GetPosition()          const throw();
    size_t   GetApproximateLength() const throw();
    
    
private:
    static const size_t InitPos = size_t(-1);
    static const size_t EndPos  = InitPos - 1;

    static size_t SizeOf(size_t nDataSize) throw();
    
    const size_t    c_nApproximateLength;
    SeqNodeL*const  c_pRoot;         
    
    size_t          m_nPos;
    uns8_t          m_arrData[1];
};

}//namespace Search
}//namespace FastFish
#include "Search/Query/Sequence.inl"

#pragma once
#include "Search/Query/Sequence.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class Query
{
    friend class Parser;
    friend class Searcher;
    friend class Optimizer;
    friend class std::auto_ptr<Query>;
    
    Query(Searcher& srch) throw();
    ~Query() throw();
    
    Query(const Query&);
    Query& operator = (const Query&);
public:
    
    Sequence* CreateSequence(size_t nArgs, ...) ffThrowAll;
    void      DeleteSequence(Sequence* pSequence) throw();

private:  
    Searcher&   m_srch;    
  
    size_t      m_nArgs;
    uns2_t*     m_arrShift;
    uns1_t*     m_arrNodes;
};


}//namespace Search
}//namespace FastFish

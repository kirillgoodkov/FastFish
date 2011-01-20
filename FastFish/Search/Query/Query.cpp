#include "Search/Query/Query.h"
#include "Search/Query/Optimizer.h"
#include "Search/Searcher.h"

using namespace std;

namespace FastFish{
namespace Search{

const char g_szFewArgs[] = "to few arguments";

Query::Query(Searcher& srch) throw():
    m_srch(srch),
    m_nArgs(0),
    m_arrShift(0),
    m_arrNodes(0)
{
}

Query::~Query() throw()
{
    delete [] m_arrShift;
    delete [] m_arrNodes;
};

void Query::DeleteSequence(Sequence* pSequence) throw()
{
    delete pSequence;
}

Sequence* Query::CreateSequence(size_t nArgs, ...) ffThrowAll
{
    if (nArgs < m_nArgs)
    {
        throw ExceptSyntax(g_szFewArgs);
    }
    
    va_list args;
    va_start(args, nArgs); 
    void* arrArgs[MaxArgumentId + 1];
    for (size_t n = 0; n < m_nArgs; ++n)
    {
        arrArgs[n] = va_arg(args, void*);
    }
    va_end(args);

    Optimizer opt(*this, arrArgs); 
    Sequence* p = opt.CreateQuery();
    m_srch.m_aOptimizer.Clear();
    return p;
}

}//namespace Search
}//namespace FastFish

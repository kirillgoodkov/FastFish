#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class Optimizer
{
    Optimizer(const Optimizer&);
    Optimizer operator = (const Optimizer&);
public:
    Optimizer(const Query& q, void* arrArgs[]) throw();
    Sequence* CreateQuery()              const ffThrowNone;

private:
    OptNode* TransformExpandSop(size_t nIdx) throw();
    
    const Query&        m_q;
    void**              m_arrArgs;

    AllocatorInvaderBB& m_a;
    OptNode*            m_pRoot;
};

}//namespace Search
}//namespace FastFish

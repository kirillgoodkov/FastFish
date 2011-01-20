#include "Memory/AllocatorInvaderBB.h"

namespace FastFish{

AllocatorInvaderBB::AllocatorInvaderBB(size_t nBlockSize, size_t nBlockAlign) throw():
    AllocatorInvader(nBlockSize, 1, nBlockAlign),
    c_nMaxSmallBlockSize(nBlockSize >> SmallBlockShift),
    m_nCount(0)
    
{
    ffAssume(c_nMaxSmallBlockSize > 0);
}

}//namespace FastFish



#include "Memory/AllocatorInvader.h"
#include "Tools/Bits.h"


namespace FastFish{

inline
size_t AlignOf(size_t nSize) throw()
{
    size_t nAlign = MaxLessOrEqualPow2(nSize);
    while (0 != (nSize%nAlign))
    {
        nAlign >>= 1;
    }
    
    return nAlign;
}

AllocatorInvader::AllocatorInvader(size_t nBlockSize, size_t nItemSize, size_t nBlockAlign) throw():
    c_nBlockSize(nBlockSize),
    c_nItemSize(nItemSize),
    c_nBlockAlign(AlignOf(nBlockAlign)),
    m_pDataNext(0),
    m_pDataEnd(0)
{
    ffAssert(MaxLessOrEqualPow2(c_nBlockAlign) == c_nBlockAlign);
    ffAssume(c_nItemSize <= c_nBlockSize);
    ffAssume(c_nBlockAlign <= c_nBlockSize);
    ffAssume(0 == (c_nBlockSize%c_nItemSize));
    ffAssume(0 == (c_nBlockSize%c_nBlockAlign));
}

AllocatorInvader::~AllocatorInvader() throw()
{
    for (ListBlocks::iterator it = m_lstBlocks.begin(); it != m_lstBlocks.end(); ++it)
    {
        delete [] *it;
    }                           
}

void AllocatorInvader::NewBlock() throw()
{
    m_lstBlocks.push_back(new byte_t[c_nBlockSize + c_nBlockAlign - 1]);
    InitPData();
}


}//namespace FastFish


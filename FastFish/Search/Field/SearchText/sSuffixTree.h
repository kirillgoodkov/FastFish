#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

template<bool bEXACT, bool bEMPTYCHECK>
class SuffixTree
{
public:
    SuffixTree(IFStream& ifs) throw();              
    size_t FindWord(const char* pWord, size_t nLen) const throw();

private:    

    uns8_t          m_nNodesCount;
    uns1_t          m_nRootCount;
    
    size_t          m_nSizeOf;
    size_t          m_nSizeOfNode;
    uns1_t          m_nSizeOfSuffix;
    
    const byte_t*   m_pTree;
    const char*     m_pSuffixs;    
};
    

}//namespace Search
}//namespace FastFish


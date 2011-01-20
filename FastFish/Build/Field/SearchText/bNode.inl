#include "Tools/OFStream.h"

namespace FastFish{
namespace Build{

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
const char* Node<bCTRL, bSENS, tit>::Suffix() const  throw()
{
    if (m_nSuffixLen > PC::NodeInPlaceSuffixLen)
    {
        return m_pSuffix;
    }
    else
    {
        return m_arrSuffix;
    }
}

}//namespace Build
}//namespace FastFish

#include "Containers/DequeRaw.h"
#include "Tools/OFStream.h"

using namespace std;

namespace FastFish{

void DequeRaw::Clear() throw()
{
    for (vector<byte_t*>::const_iterator it = m_vecBlocks.begin();
         it != m_vecBlocks.end();
         ++it)
    {
        ffAssert(*it);
        delete [] *it;
    }            
     
    m_nCount = 0;    
}


}//namespace FastFish


#include "Build/Documents/bDocumentsNoRanks.h"
#include "Tools/OFStream.h"

namespace FastFish{
namespace Build{

void DocumentsNoRanks::Store(OFStream& file) const ffThrowAll
{
    file << uns1_t(0)//version
         << m_didNext
         << rank_t(1);

    file.BagBegin(sizeof(docid_t));
    file << m_didNext;
    file.BagCommit();
         
    file.BagBegin(sizeof(docid_t));
    file << m_didNext;
    file.BagCommit();
    
    file.BagBegin(sizeof(rank_t));
    file << rank_t(0);
    file.BagCommit();
}

}//namespace Build
}//namespace FastFish

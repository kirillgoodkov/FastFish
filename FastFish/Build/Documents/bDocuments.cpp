#include "Build/Documents/bDocuments.h"
#include "Build/Documents/bDocumentsNoRanks.h"
#include "Build/Documents/bDocumentsRanked.h"
#include "Common/Templates.h"

namespace FastFish{
namespace Build{

Documents* CreateDocuments(const TplIndex& tpl) throw()
{
    ffAssumeStatic(sizeof(rank_t) <= 2);
    
    if (1 == tpl.nRanksCount)
        return new DocumentsNoRanks();
    else
        return new DocumentsRanked(tpl);
}

}//namespace Build
}//namespace FastFish


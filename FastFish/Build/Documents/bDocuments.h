#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Build{

struct Documents
{
    virtual ~Documents()                        throw()     {};
    
    virtual docid_t Add(rank_t rank)            throw()     = 0;    
    virtual docid_t Count()               const throw()     = 0;
    
    virtual void    Finalize()                  throw()     = 0;
    virtual void    Store(OFStream& file) const ffThrowAll  = 0;
};

Documents* CreateDocuments(const TplIndex& tpl) throw();

}//namespace Build
}//namespace FastFish


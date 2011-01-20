#pragma once
#include "Build/Documents/bDocuments.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

class DocumentsNoRanks:
    public Documents
{
    DocumentsNoRanks(const DocumentsNoRanks&);
    DocumentsNoRanks& operator = (const DocumentsNoRanks&);
public:
    DocumentsNoRanks()                          throw()     : m_didNext(0) {}

    virtual docid_t Add(rank_t /*rank*/)        throw()     {return m_didNext++;}
    virtual docid_t Count()               const throw()     {return m_didNext;}
    
    virtual void    Finalize()                  throw()     {}
    virtual void    Store(OFStream& file) const ffThrowAll;

    docid_t TransformForw(docid_t did)    const throw()     {return did;}
    docid_t TransformBack(docid_t did)    const throw()     {return did;}
    
private:        
    
    docid_t m_didNext;
};

}//namespace Build
}//namespace FastFish


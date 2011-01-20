#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

struct Field
{
    virtual ~Field() throw(){};
    
    virtual void Load(IFStream& ifs) throw() = 0;
};

struct FieldSearch:
    public Field
{
    virtual bool   FindPostings(handle_t arr[], size_t& nCount, void* arg)      throw() = 0;
    virtual void   PsFree(handle_t h)                                           throw() = 0;
    virtual void   PsReset(handle_t h)                                          throw() = 0;    

    virtual bool   PsGetNext(handle_t h, Document& doc)                         throw() = 0;    
    virtual bool   PsGetNextFrom(handle_t h, Document docFrom, Document& doc)   throw() = 0;    
    
    virtual bool   PsIsContain(handle_t h, Document doc)                        throw() = 0;
    virtual size_t PsGetCardinality(handle_t h)                           const throw() = 0;    
};

struct FieldData:
    public Field
{
    virtual const void* Get(Document doc, datasize_t* pSize = 0) const throw() = 0;    
};

Field* CreateField(const TplFieldDataFix& tpl, Documents* pDocs) throw();
Field* CreateField(const TplFieldDataVar& tpl, Documents* pDocs) throw();
Field* CreateField(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs) throw();

}//namespace Search
}//namespace FastFish

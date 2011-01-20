#pragma once
#include "Search/Field/sField.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class FieldDataFix:
    public FieldData
{
    FieldDataFix(const FieldDataFix&);
    FieldDataFix& operator = (const FieldDataFix&);    
public:
    FieldDataFix(const TplFieldDataFix& tpl)                       throw();
    virtual ~FieldDataFix()                                        throw() {};
    
    virtual void        Load(IFStream& ifs)                        throw();
    virtual const void* Get(Document doc, datasize_t* pSize) const throw();
private:
    const TplFieldDataFix& c_tpl;
    
    struct Segment
    {
        docid_t         nCount;
        const byte_t*   pData;    
    };
    std::vector<Segment>    m_vecSeg;
};

}//namespace Search
}//namespace FastFish

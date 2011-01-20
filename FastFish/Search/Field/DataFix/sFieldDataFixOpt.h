#pragma once
#include "Search/Field/sField.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class FieldDataFixOpt:
    public FieldData
{
    FieldDataFixOpt(const FieldDataFixOpt&);
    FieldDataFixOpt& operator = (const FieldDataFixOpt&);    
public:
    FieldDataFixOpt(const TplFieldDataFix& tpl)                     throw();
    virtual ~FieldDataFixOpt()                                      throw() {};
                                                                 
    virtual void        Load(IFStream& ifs)                         throw();
    virtual const void* Get(Document doc, datasize_t* pSize)  const throw();
private:
    const TplFieldDataFix&  c_tpl;
    
    struct Segment
    {
        docid_t         nCount;
        const byte_t*   pData;
        const docid_t*  pIdx;  
    };
    std::vector<Segment>    m_vecSeg;
};

}//namespace Search
}//namespace FastFish

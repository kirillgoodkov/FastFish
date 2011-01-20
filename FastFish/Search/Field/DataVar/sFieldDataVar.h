#pragma once
#include "Search/Field/sField.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class FieldDataVar:
    public FieldData
{
    FieldDataVar(const FieldDataVar&);
    FieldDataVar& operator = (const FieldDataVar&);    
public:
    FieldDataVar(const TplFieldDataVar& tpl)                        throw()     : c_tpl(tpl) {}
    virtual ~FieldDataVar()                                         throw();
    
    virtual void        Load(IFStream& ifs)                         throw();
    virtual const void* Get(Document doc, datasize_t* pSize)  const throw();

private:
    struct Segment
    {
        virtual ~Segment() throw() {};
        virtual const void* Get(docid_t did, datasize_t* pSize) const throw() = 0;
    };
    
    template<typename POS_TYPE, typename SIZE_TYPE>
    class SegmentT:
        public Segment
    {
    public:
        SegmentT(IFStream& ifs)                                       throw();
        virtual const void* Get(docid_t did, datasize_t* pSize) const throw();
        
    private:                
        static const size_t BlockSize = (FC::DataVarBlockSize - sizeof(POS_TYPE))/sizeof(SIZE_TYPE);
        struct Block
        {
            POS_TYPE    nBase;
            SIZE_TYPE   arrSizes[BlockSize];
        };
        
        const byte_t* m_pData;    
        const Block*  m_pIdx;
        
    };

    const TplFieldDataVar&  c_tpl;
        
    std::vector<Segment*>   m_vecSeg;
};

}//namespace Search
}//namespace FastFish

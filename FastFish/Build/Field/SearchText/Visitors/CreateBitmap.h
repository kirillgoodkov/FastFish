#pragma once
#include "Memory/ArrayPtr.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

template<typename DOCUMENTS>
class CreateBitmap
{
    CreateBitmap(const CreateBitmap&);
    CreateBitmap& operator = (const CreateBitmap&);
public:
    CreateBitmap(const DOCUMENTS* pDocs) throw();
    
    template<typename _DOCUMENTS> friend 
    OFStream& operator << (OFStream& file, const CreateBitmap<_DOCUMENTS>& vis) ffThrowAll;

    void operator () (docid_t val) throw();    

    docid_t arrDocuments[2];
private:    
    
    const DOCUMENTS*const   c_pDocs;
    const size_t            c_nSize;
    
    ArrayPtr<bmpword_t>     m_arr;  
    size_t                  m_nStore;  
    
};

}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/Visitors/CreateBitmap.inl"

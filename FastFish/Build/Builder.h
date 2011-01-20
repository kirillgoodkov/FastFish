#pragma once
#include "Build/Field/bField.h"
#include "Memory/ArrayPtr.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

class Builder
{
    Builder(const Builder&);
    Builder& operator = (const Builder&);
public:
    Builder(const TplIndex& tpl)                  throw();
    ~Builder()                                    throw();
    
    void        AddField(fldid_t fid, const void* pData, datasize_t nSize = 0) throw();
    docid_t     CommitDocument(rank_t rank = 0)   throw();

    void        Finalize()                        throw();
    void        Store(const char* pszFile)  const ffThrowAll;
    
    docid_t     DocumentsCount()            const throw();

private:
    typedef std::auto_ptr<Field>    FieldPtr;
    typedef ArrayPtr<FieldPtr>      ArrayPtrField;

    const TplIndex&             c_tpl;

    std::auto_ptr<Documents>    m_pDocs;
    TextAllocator               m_a;
    size_t                      m_nFields;
    ArrayPtrField               m_arrFields;
};

}//namespace Build
}//namespace FastFish


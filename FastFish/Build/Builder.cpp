#include "Build/Builder.h"
#include "Build/Field/bField.h"
#include "Build/Documents/bDocuments.h"
#include "Tools/OFStream.h"
#include "Common/TemplateLoaders.h"
#include "Common/Consts.h"

using namespace std;

namespace FastFish{
namespace Build{

Builder::Builder(const TplIndex& tpl) throw():
    c_tpl(tpl),
    m_pDocs(CreateDocuments(tpl)),
    m_nFields(tpl.FieldsCount()),
    m_arrFields(m_nFields)
{
    ffAssumeUser(0 < c_tpl.nRanksCount && c_tpl.nRanksCount <= MaxRank + 1, "invalid template");
    ffAssumeUser(0 == c_tpl.nFieldSearchTextCount || c_tpl.arrFieldSearchText, "invalid template");
    ffAssumeUser(0 == c_tpl.nFieldDataFixCount || c_tpl.arrFieldDataFix, "invalid template");
    ffAssumeUser(0 == c_tpl.nFieldDataVarCount || c_tpl.arrFieldDataVar, "invalid template");
    ffAssumeUser(0 < m_nFields && m_nFields <= MaxFieldId + 1, "invalid template");

    size_t n = 0;
    for (const TplFieldDataFix* p = c_tpl.arrFieldDataFix; p < c_tpl.arrFieldDataFix + c_tpl.nFieldDataFixCount; ++p, ++n) 
        m_arrFields[n].reset(CreateFieldDataFix(c_tpl, *p, m_pDocs.get()));

    for (const TplFieldDataVar* p = c_tpl.arrFieldDataVar; p < c_tpl.arrFieldDataVar + c_tpl.nFieldDataVarCount; ++p, ++n) 
        m_arrFields[n].reset(CreateFieldDataVar(c_tpl, *p, m_pDocs.get()));
    
    for (const TplFieldSearchText* p = c_tpl.arrFieldSearchText; p < c_tpl.arrFieldSearchText + c_tpl.nFieldSearchTextCount; ++p, ++n) 
        m_arrFields[n].reset(CreateFieldSearchText(c_tpl, *p, m_pDocs.get(), m_a));
}

Builder::~Builder() throw()
{
}

void Builder::AddField(fldid_t fid, const void* pData, datasize_t nSize) throw()
{
    ffAssumeUser(fid < m_nFields, "invalid argument cid");
    ffAssumeUser(pData, "invalid argument pData");
    
    m_arrFields[fid]->Add(pData, nSize);    
}

docid_t Builder::CommitDocument(rank_t rank) throw()
{
    ffAssertUser(m_pDocs->Count() < MaxDocumentId + 1, "document count exceeded");        
    ffAssumeUser(rank < c_tpl.nRanksCount, "invalid rank argument");
    
    const FieldPtr* ppEnd = m_arrFields.Get() + m_nFields;
    for (FieldPtr* pp = m_arrFields.Get(); pp < ppEnd; ++pp)
    {
        (*pp)->Commit(rank);
    }
    docid_t did = m_pDocs->Add(rank);
    
    m_a.aRMap.Clear();
    m_a.aRStr.Clear();        
    
    return did;
}

void Builder::Finalize() throw()
{
    m_pDocs->Finalize();
    const FieldPtr* ppEnd = m_arrFields.Get() + m_nFields;
    for (FieldPtr* pp = m_arrFields.Get(); pp < ppEnd; ++pp)
    {
        (*pp)->Finalize();
    }        
}

void Builder::Store(const char* pszFile) const ffThrowAll
{
    OFStream file (pszFile);
    file << FC::Signature << FC::VersionMajor << FC::VersionMinor;
    file.BagBegin();
    file << FC::Signature;
    file.BagCommit();
    
    file << c_tpl;

    ffLog(lsInfoFileSummary, "\n");

    m_pDocs->Store(file);

    const FieldPtr* ppEnd = m_arrFields.Get() + m_nFields;
    for (const FieldPtr* pp = m_arrFields.Get(); pp < ppEnd; ++pp)
    {
        ffLog(lsInfoFileSummary, "field: %u\n", pp - m_arrFields.Get());
        (*pp)->Store(file);
    }    
    
    file.Finalize();        
}

docid_t Builder::DocumentsCount() const throw()
{
    return m_pDocs->Count();
}

}//namespace Build
}//namespace FastFish

#include "Search/Searcher.h"
#include "Search/Layers.h"
#include "Search/Documents/sDocuments.h"
#include "Search/Field/sField.h"
#include "Search/Query/Parser.h"
#include "Search/Query/Query.h"
#include "Tools/IFStream.h"
#include "Tools/MemMap.h"
#include "Tools/MemManager.h"
#include "Tools/Tools.h"
#include "Common/Consts.h"

using namespace std;

#define ffCheck(EXPR, STR)\
if (!(EXPR))\
{\
    throw ExceptFileFormat(STR);\
}

namespace FastFish{
namespace Search{

Searcher::Searcher() throw():
    m_mman(lisCount),
    m_aOptimizer(PC::SearcherOptimizerAllocSize, sizeof(void*))
{
}

Searcher::~Searcher() throw()
{
    for(vector<MemMap*>::iterator it = m_vecMaps.begin(); it != m_vecMaps.end(); ++it)
    {
        delete *it;
    }
}

segid_t Searcher::Load(const char* pszFile) ffThrowAll
{
    ffAssertUser(m_vecMaps.size() < MaxSegId + 1, "segment count exceeded");

    m_vecMaps.push_back(new MemMap(pszFile));
    IFStream ifs(*m_vecMaps.back(), m_mman);

    uns8_t nSignature; ifs >> nSignature;
    ffCheck(FC::Signature == nSignature, "invalid file");
    uns4_t nVerMajor, nVerMinor;   
    ifs >> nVerMajor >> nVerMinor;

    if (FC::VersionMajor != nVerMajor || FC::VersionMinor < nVerMinor)
    {
        throw ExceptFileFormat("unsupported file version");
    }        
    
    const uns8_t* pSignature = ifs.GetBag<uns8_t>(liDocuments);
    ffTouch(pSignature);
    ffDebugOnly(ffCheck(FC::Signature == *pSignature, "invalid file"));
    
    if (1 == m_vecMaps.size())
    {
        m_tpl.Load(ifs);
        m_pDocs.reset(new Documents(m_tpl));
        m_arrFields.Alloc(m_tpl.FieldsCount());
        size_t nIdx = 0;
        for (size_t n = 0; n < m_tpl.nFieldDataFixCount; ++n)
            m_arrFields[nIdx++].reset(CreateField(m_tpl.arrFieldDataFix[n], m_pDocs.get()));
        for (size_t n = 0; n < m_tpl.nFieldDataVarCount; ++n)
            m_arrFields[nIdx++].reset(CreateField(m_tpl.arrFieldDataVar[n], m_pDocs.get()));
        for (size_t n = 0; n < m_tpl.nFieldSearchTextCount; ++n)
            m_arrFields[nIdx++].reset(CreateField(m_tpl, m_tpl.arrFieldSearchText[n], m_pDocs.get()));
    }
    else
    {
        TplIndexLoader ldr;
        ldr.Load(ifs);
        ffCheck(m_tpl == ldr, "incompatible segments");
    }                                            
    
    m_pDocs->Load(ifs);    
    size_t nIdx = 0;
    for (size_t n = 0; n < m_tpl.nFieldDataFixCount; ++n)
        m_arrFields[nIdx++]->Load(ifs);
    for (size_t n = 0; n < m_tpl.nFieldDataVarCount; ++n)
        m_arrFields[nIdx++]->Load(ifs);
    for (size_t n = 0; n < m_tpl.nFieldSearchTextCount; ++n)
        m_arrFields[nIdx++]->Load(ifs);
    
    return segid_t(m_vecMaps.size() - 1);
}

bool Searcher::LockCritical(size_t nMaxSize) const throw()
{
    return m_mman.Lock(liData, nMaxSize);
}

void Searcher::Prefetch(size_t nMaxSize) const throw()
{
    m_mman.Prefetch(nMaxSize);
}

Query* Searcher::CreateQuery(const char* pszQuery) ffThrowAll
{
    Parser parser(m_tpl, pszQuery);
    return parser.CreateQuery(*this);
}

void Searcher::DeleteQuery(Query* pQuery) throw()
{
    delete pQuery;
}

const void* Searcher::GetData(fldid_t fid, Document doc, datasize_t* pSize) const throw()
{
    ffAssertUser(doc.m.sid < m_vecMaps.size(), "invalid segment id");
    ffAssertUser(fid < m_tpl.FieldsCount(), "invalid Field id");
    ffAssertUser(dynamic_cast<FieldData*>(m_arrFields[fid].get()), "this Field don't support data retrieving");
    return static_cast<FieldData*>(m_arrFields[fid].get())->Get(doc, pSize);
}

}//namespace Search
}//namespace FastFish

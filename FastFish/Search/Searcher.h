#pragma once
#include "Memory/AllocatorInvaderBB.h"
#include "Memory/ArrayPtr.h"
#include "Tools/MemManager.h"
#include "Common/TemplateLoaders.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class Searcher
{
    friend class Optimizer;
    friend class Query;
    
    Searcher(const Searcher&);
    Searcher& operator = (const Searcher&);
public:
    Searcher()  throw();
    ~Searcher() throw();
    
    segid_t Load(const char* pszFile)                                           ffThrowAll;
    
    bool    LockCritical(size_t nMaxSize = size_t(-1))                    const throw();
    void    Prefetch(size_t nMaxSize = size_t(-1))                        const throw();

    Query*  CreateQuery(const char* pszQuery)                                   ffThrowAll;
    void    DeleteQuery(Query* pQuery)                                          throw();
    
    const void* GetData(fldid_t fid, Document doc, datasize_t* pSize = 0) const throw();    

private:    
    typedef std::auto_ptr<Documents>    IDocumentsPtr;
    typedef std::auto_ptr<Field>        IFieldPtr;
    
    TplIndexLoader          m_tpl;        
    std::vector<MemMap*>    m_vecMaps;    
    MemManager              m_mman;
    IDocumentsPtr           m_pDocs;                        
    ArrayPtr<IFieldPtr>     m_arrFields;
    AllocatorInvaderBB      m_aOptimizer;    
};

}//namespace Search
}//namespace FastFish

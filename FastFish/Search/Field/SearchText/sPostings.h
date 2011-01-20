#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

struct PsContext
{
    size_t  nSize; 
    docid_t did0;
    
    union
    {
        struct
        {
            docid_t             did1;
            const bmpword_t*    p;
        }   bmp;
        struct
        {
            docid_t             nBlocks;
            const uns1_t*       pList;          
        }   cmpr;
        struct
        {
            docid_t             pad;
            const docid_t*      p;
        }   lst;
    };               
};

struct PsCursor
{
    Document    doc;        //inpl, fix, cmpr:      prev value
                            //bmp:      -
    size_t      nPos;       
    docid_t     didLastCR;  //last did for current rank, document with did <= (didLastCR - 1) has different rank
    
    void UpdateRank(const Documents* pDocs) throw();
};

struct Postings
{
    PsContext   arrCtx[1];
    size_t      nCardinality;
    size_t      nCtxCount;
    size_t      nCurCount;
    PsCursor    arrCur[1];
    
    static size_t SizeOf(size_t nSegCount) throw();
    static Postings* New(size_t nSegCount, AllocatorSwapper& a) throw();

    void Delete(size_t nSegCount, AllocatorSwapper& a) throw();
    void AddContext(segid_t sid, const PsContext& ctx) throw();
    void InsertCursor(PsCursor curNew) throw();
    void ResetCursors() throw();
};         
    
}//namespace Search
}//namespace FastFish

#include "Search/Field/SearchText/sPostings.inl"

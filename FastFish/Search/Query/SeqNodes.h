#pragma once
#include "Search/Documents/sDocuments.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

#pragma pack(push, 1)
                            //rnk*sid*did*****
const uns8_t DocumentNull = 0xFFFF00007FFFFFFFull;
const uns8_t DocumentEOF  = 0xFFFF0000FFFFFFFFull;
const uns8_t DocumentMax  = 0xFFFFFFFFFFFFFFFFull;

enum SeqNodeType
{
    //Dictionary nodes
    sntDAnd,
    sntDOr,
    sntDLeaf,
    sntDLeafNeg,
    //List nodes
    sntLAnd,
    sntLOr,
    sntLLeaf,
    sntLFull
};

struct SeqNode
{
    SeqNodeType snt;
    int         pad;
    
    void FreePostings() throw();
    void Reset()        throw();
};

//---------------------------------------------------------------------------

struct SeqNodeD: public SeqNode
{
    bool Find(const Document& doc) throw();
};

struct SeqNodeDAndOr: public SeqNodeD
{
    size_t          nCount;
    SeqNodeD*       arrNodes[1];

    SeqNodeD*const* End() const throw() {return arrNodes + nCount;}
    
    static size_t SizeOf(size_t nCount) throw();
};

struct SeqNodeDLeaf: public SeqNodeD
{
    FieldSearch*    pField;
    handle_t        hPostings;
};

//---------------------------------------------------------------------------

struct SeqNodeL: public SeqNode
{
    Document  doc;
    
    void Pop(Document docFrom = DocumentMax) throw();   
};

struct SeqNodeLAnd: public SeqNodeL
{
    size_t      nCountL;
    size_t      nCountD;
    SeqNode*    arrNodes[1];        

    SeqNode*const*  End()    const throw()  {return arrNodes + nCountL + nCountD;}

    SeqNodeL**      BeginL()       throw()  {return reinterpret_cast<SeqNodeL**>(arrNodes);}
    SeqNodeL*const* BeginL() const throw()  {return reinterpret_cast<SeqNodeL*const*>(arrNodes);}
    SeqNodeL*const* EndL()   const throw()  {return reinterpret_cast<SeqNodeL*const*>(arrNodes) + nCountL;}

    SeqNodeD*const* BeginD() const throw()  {return reinterpret_cast<SeqNodeD*const*>(arrNodes) + nCountL;}
    SeqNodeD*const* EndD()   const throw()  {return BeginD() + nCountD;}

    bool Filter() const throw();
    
    static size_t SizeOf(size_t nCount) throw();
};

struct SeqNodeLOr: public SeqNodeL
{
    size_t          nCount;
    SeqNodeL*       arrNodes[1];        

    SeqNodeL*const* End() const throw() {return arrNodes + nCount;}
    
    static size_t SizeOf(size_t nCount) throw();
};

struct SeqNodeLLeaf: public SeqNodeL
{
    FieldSearch*    pField;
    handle_t        hPostings;        
};

struct SeqNodeLFull: public SeqNodeL
{
    Documents*           pDocs;
    Documents::Context   ctx;
};

#pragma pack(pop)


}//namespace Search
}//namespace FastFish
#include "Search/Query/SeqNodes.inl"

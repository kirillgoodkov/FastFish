#pragma once
#include "Search/Query/Lexer.h"
#include "Search/Query/AstNodes.h"
#include "Common/Common.h"

namespace FastFish{
namespace Search{

class Parser:
    private Lexer
{
    Parser(const Parser&);
    Parser& operator = (const Parser&);
public:
    Parser(const TplIndexLoader& tpl, const char* pszQuery) ffThrowAll;
    Query* CreateQuery(Searcher& srch)                      ffThrowNone;    
    
private:   
    uns1_t      Expr2()                                 ffThrowAll;
    uns1_t      Expr1()                                 ffThrowAll;
    uns1_t      Expr0()                                 ffThrowAll;
                                   
    fldid_t     ExpectFieldId(const uns1_t* pCompBmp)   ffThrowAll;
    argid_t     ExpectArgumentId()                      ffThrowAll;
    uns1_t      NewNodeFix()                            ffThrowAll;
            
    void        PutDownNegs(size_t n)                   throw();
    void        MergeDomain(size_t nFx, AstNodeC& node) throw();
    uns1_t      CopyFix2C(size_t n)                     throw();

    const TplIndexLoader& c_tpl;
    
    AstNodeFix  m_arrNode[SizeUns1];
    size_t      m_nNext;
    
    uns2_t      m_arrShift[SizeUns1];       //relative node positions in m_arrNodeC
    size_t      m_nNextShift;
    uns1_t      m_arrNodeC[SizeUns1 * 3];   //C - compact (form)
    size_t      m_nNextC;
    
    size_t      m_nMaxArg;
    size_t      m_nArgCount;    
    
};

}//namespace Search
}//namespace FastFish


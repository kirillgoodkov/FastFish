#include "Search/Query/Parser.h"
#include "Search/Query/Query.h"

using namespace std;


namespace FastFish{
namespace Search{

const char g_szEnding[]        = "invalid expression ending";
const char g_szBrRd[]          = "round braces mismatch";
const char g_szFieldId[]       = "Field index expected";
const char g_szComma[]         = "comma expected";
const char g_szArgId[]         = "argument index expected";
const char g_szBrCrOpen[]      = "open curly brace expected";
const char g_szBrCrClose[]     = "close curly brace expected";
const char g_szSop[]           = "invalid subset operation";
const char g_szFieldIncomp[]   = "Field is incompatible with this operation";
const char g_szArgIdx[]        = "invalid argument id";
const char g_szSopLimit[]      = "too many items in expression";

//---------------------------------------------------------------------------

ffForceInline
fldid_t Parser::ExpectFieldId(const uns1_t* pCompBmp) ffThrowAll
{
    size_t n = ExpectNum(g_szFieldId);
    if (!GetBit(pCompBmp, n))        
        Error(g_szFieldIncomp);
    return fldid_t(n);
}

ffForceInline
argid_t Parser::ExpectArgumentId() ffThrowAll
{
    size_t n = ExpectNum(g_szArgId);
    ++m_nArgCount;
    if (MaxArgumentId < m_nArgCount || MaxArgumentId < n )
        Error(g_szArgIdx);
    m_nMaxArg = max(m_nMaxArg, n);
    return argid_t(n);
}

ffForceInline
uns1_t Parser::NewNodeFix() ffThrowAll
{
    if (ffCountOf(m_arrNode) <= m_nNext)
    {
        Error(g_szSopLimit);            
    }        
    return uns1_t(m_nNext++);
}


//---------------------------------------------------------------------------

Parser::Parser(const TplIndexLoader& tpl, const char* pszQuery) ffThrowAll:
    Lexer(pszQuery),
    c_tpl(tpl),
    m_nNext(0),    
    m_nNextShift(0),
    m_nNextC(0),
    m_nMaxArg(0),
    m_nArgCount(0)
{
    size_t nRoot = Expr2();

    if (tkEnd != Lookup())
        Error(g_szEnding);
        
    PutDownNegs(nRoot);        
    ffVerify(0 == CopyFix2C(nRoot));
}

Query* Parser::CreateQuery(Searcher& srch) ffThrowNone
{
    auto_ptr<Query> pQuery(new Query(srch));
    if (1 != m_nNextShift)
    {
        pQuery->m_arrShift = new uns2_t[m_nNextShift - 1];    
        memcpy(pQuery->m_arrShift, m_arrShift + 1, sizeof(uns2_t) * (m_nNextShift - 1));
    }
    pQuery->m_arrNodes = new uns1_t[m_nNextC];
    memcpy(pQuery->m_arrNodes, m_arrNodeC, sizeof(uns1_t) * m_nNextC);

    pQuery->m_nArgs = m_nMaxArg + 1;
    
    return pQuery.release();
}
uns1_t Parser::Expr2() ffThrowAll
{
    uns1_t nLeft = Expr1();
    
    while (Match(tkEopOr))
    {
        uns1_t n         = NewNodeFix();
        AstNodeFix& node = m_arrNode[n];
        node.nFlags      = antNodeOr;
        node.arrArg[0]   = nLeft;
        node.arrArg[1]   = Expr2();          
                       
        nLeft            = n;
    }           
    
    return nLeft;
}

uns1_t Parser::Expr1() ffThrowAll
{
    uns1_t nLeft = Expr0();
    
    while (Match(tkEopAnd))
    {
        uns1_t n         = NewNodeFix();
        AstNodeFix& node = m_arrNode[n];
        node.nFlags      = antNodeAnd;
        node.arrArg[0]   = nLeft;
        node.arrArg[1]   = Expr1();
        
        nLeft            = n;
    }
    
    return nLeft;
}


uns1_t Parser::Expr0() ffThrowAll
{
    bool bFirst = false;
    switch (Lookup()) 
    {
        case tkEopNot: Next();
        {
            uns1_t n = Expr0();
            m_arrNode[n].FlipNeg();
            return n;
        }            
        case tkBrRdOpen: Next();
        {
            uns1_t n = Expr2();
            Expect(tkBrRdClose, g_szBrRd);
            return n;
        }            
        case tkSopContainAll:   bFirst = true;
        case tkSopContainSome:  Next();
        {
            uns1_t n         = NewNodeFix();              
            AstNodeFix& node = m_arrNode[n];
            node.nFlags      = uns1_t(bFirst ? antLeafA : antLeafS);
            node.arrArg[0]   = ExpectFieldId(c_tpl.arrFieldAS);
            Expect(tkComma, g_szComma);
            node.arrArg[1]   = ExpectArgumentId();          
            
            return n;
        }            
        default:
            Error(g_szSop);
            ffAssume(false); 
    }    
    ffReturnNop;
}

void Parser::PutDownNegs(size_t n) throw()
{
    AstNodeFix& node = m_arrNode[n];
    
    if (node.IsLeaf())
    {
        if (node.IsNeg())
        {
            node.FlipType();
        }
    }
    else
    {
        if (node.IsNeg())
        {
            node.FlipNeg();
            node.FlipType();
            m_arrNode[node.arrArg[0]].FlipNeg();
            m_arrNode[node.arrArg[1]].FlipNeg();
        }
        
        PutDownNegs(node.arrArg[0]);
        PutDownNegs(node.arrArg[1]);
    }    
}

void Parser::MergeDomain(size_t nFx, AstNodeC& node) throw()
{
    AstNodeFix& nodeFx = m_arrNode[nFx];

    if(node.nFlags == nodeFx.nFlags)
    {
        MergeDomain(nodeFx.arrArg[0], node);
        MergeDomain(nodeFx.arrArg[1], node);
    }
    else
    {
        ffAssume(nFx <= MaxUns1);
        node.arrChilds[++node.nLastIdx] = uns1_t(nFx);
    }
}

uns1_t Parser::CopyFix2C(size_t nFx) throw()
{
    ffAssume(m_nNextShift < ffCountOf(m_arrShift));
    
    size_t nC      = m_nNextShift++;
    size_t nShift  = m_nNextC;
    m_arrShift[nC] = uns2_t(nShift);
    
    const AstNodeFix& nodeFx = m_arrNode[nFx];
    switch (nodeFx.GetType())
    {
        case antNodeAnd:
        case antNodeOr:
        {
            AstNodeC* __restrict pNode = reinterpret_cast<AstNodeC*>(&m_arrNodeC[nShift]);
            pNode->nFlags   = nodeFx.nFlags;
            pNode->nLastIdx = uns1_t(-1);
            MergeDomain(nodeFx.arrArg[0], *pNode);
            MergeDomain(nodeFx.arrArg[1], *pNode);
            m_nNextC += sizeof(*pNode) + pNode->nLastIdx;
            
            for (size_t n = 0; n <= pNode->nLastIdx; ++n)
            {
                pNode->arrChilds[n] = CopyFix2C(pNode->arrChilds[n]);
            }
            break;
        }
        case antLeafA:
        case antLeafS:
        {
            AstLeafSop* __restrict pNode = reinterpret_cast<AstLeafSop*>(&m_arrNodeC[nShift]);
            pNode->nFlags = nodeFx.nFlags;
            pNode->nFldId = nodeFx.arrArg[0];
            pNode->nArgId = nodeFx.arrArg[1];
            m_nNextC += sizeof(*pNode);
            break;
        }
        default:
            ffAssume(false);
    }
    
    ffAssume(m_nNextC <= ffCountOf(m_arrNodeC));
    ffAssume(nC <= MaxUns1);
    return uns1_t(nC);
}


}//namespace Search
}//namespace FastFish

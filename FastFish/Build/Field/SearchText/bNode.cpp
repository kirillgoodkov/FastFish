#include "Build/Field/SearchText/bNode.h"
#include "Build/Field/bField.h"
#include "Tools/Bits.h"

using namespace std;

namespace FastFish{
namespace Build{

template<bool bCTRL, bool bSENS, TextIndexType tit> ffForceInline
char* Node<bCTRL, bSENS, tit>::Suffix() throw()
{
    if (m_nSuffixLen > PC::NodeInPlaceSuffixLen)
    {
        return m_pSuffix;
    }
    else
    {
        return m_arrSuffix;
    }
}

template<bool bCTRL, bool bSENS, TextIndexType tit>
void Node<bCTRL, bSENS, tit>::Split(wordsize_t nFrom, TextAllocator& a) throw()
{
    MapTypePd mapNew;
    Node& child       = mapNew.val.AddOrGet(Suffix()[nFrom], a.aMap);
    child.m_nSuffixLen = m_nSuffixLen - nFrom - 1;
    ffAssume(nFrom < m_nSuffixLen);
    if (PC::NodeInPlaceSuffixLen < child.m_nSuffixLen)
    {
        child.m_pSuffix = m_pSuffix + nFrom + 1;
    }
    else
    {
        memcpy(child.m_arrSuffix, Suffix() + nFrom + 1, child.m_nSuffixLen);
    }
    
    if (Static(titExact == tit))
    {
        child.m_setPostings = m_setPostings;
    }
    else
    {
        child.m_setPostings.CopyFrom(m_setPostings, a.aSet);
    }
    
    child.m_mapChilds = m_mapChilds;
    m_mapChilds       = mapNew.val;

    if (PC::NodeInPlaceSuffixLen < m_nSuffixLen && nFrom <= PC::NodeInPlaceSuffixLen)
    {
        memcpy(m_arrSuffix, m_pSuffix, nFrom);
    }
    m_nSuffixLen = nFrom;
}


//---------------------------------------------------------------------------
//recursive

template<bool bCTRL, bool bSENS, TextIndexType tit>
void Node<bCTRL, bSENS, tit>::AddWord(ContextAW& ctx) throw()
{
    wordsize_t nEqualCount = EqualCount(Suffix(), ctx.pWord, min(ctx.nWordLen, m_nSuffixLen));
    
    if (nEqualCount < m_nSuffixLen)
    {
        Split(nEqualCount, *ctx.pA);
    }

    if (Static(titExact == tit))
    {
        if (nEqualCount < ctx.nWordLen)
        {
            if (m_setPostings.IsEmpty() && m_mapChilds.IsEmpty())
            {
                if (PC::NodeInPlaceSuffixLen < ctx.nWordLen)
                {
                    m_pSuffix = static_cast<char*>(ctx.pA->aStr.Alloc(ctx.nWordLen));
                }
                m_nSuffixLen = ctx.nWordLen;
                memcpy(Suffix(), ctx.pWord, ctx.nWordLen);

                m_setPostings.Insert(ctx.did, ctx.pA->aSet);
            }
            else
            {
                char ch = ctx.pWord[nEqualCount];
                wordsize_t nCut = nEqualCount + 1;
                ffAssume(nCut <= ctx.nWordLen);
                ctx.pWord   += nCut;
                ctx.nWordLen = ctx.nWordLen - nCut;
                m_mapChilds.AddOrGet(ch, ctx.pA->aMap).AddWord(ctx);
            }                                    
        } 
        else
        {
            if (!m_setPostings.IsExist(ctx.did))
            {
                m_setPostings.Insert(ctx.did, ctx.pA->aSet);
            }      
        }
    }
    else//titPrefix|titInfix
    {
        if (ctx.nSkipLen <= nEqualCount && !m_setPostings.IsExist(ctx.did))
        {
            m_setPostings.Insert(ctx.did, ctx.pA->aSet);    
        }        

        if (0 == ctx.nWordLen)
        {
            ffAssume(0 == m_nSuffixLen);
            return;        
        }        
        
        if (nEqualCount < ctx.nWordLen)
        {
            ffAssume(nEqualCount == m_nSuffixLen);
            if (m_setPostings.HasOneItem() && m_mapChilds.IsEmpty())
            {
                if (PC::NodeInPlaceSuffixLen < ctx.nWordLen)
                {
                    m_pSuffix = static_cast<char*>(ctx.pA->aStr.Alloc(ctx.nWordLen));
                }
                m_nSuffixLen = ctx.nWordLen;
                memcpy(Suffix(), ctx.pWord, ctx.nWordLen);
            }
            else
            {
                char ch = ctx.pWord[nEqualCount];
                wordsize_t nCut = nEqualCount + 1;
                ffAssume(nCut <= ctx.nWordLen);
                ctx.pWord   += nCut;
                ctx.nWordLen = ctx.nWordLen - nCut;
                ctx.nSkipLen = ctx.nSkipLen - nCut;
                
                m_mapChilds.AddOrGet(ch, ctx.pA->aMap).AddWord(ctx);
            }
        }
    }        
}


template<bool bCTRL, bool bSENS, TextIndexType tit>
void Node<bCTRL, bSENS, tit>::Merge(Node& other, TextAllocator& a) throw()
{
    wordsize_t nEqualCount = EqualCount(Suffix(), other.Suffix(), min(m_nSuffixLen, other.m_nSuffixLen));
    
    if (nEqualCount < m_nSuffixLen)
    {
        Split(nEqualCount, a);
    }

    if (nEqualCount < other.m_nSuffixLen)
    {
        other.Split(nEqualCount, a);
    }
    
    m_setPostings.Merge(other.m_setPostings, a.aSet);
    ffAssume(m_nSuffixLen == other.m_nSuffixLen);
    ffAssert(m_nSuffixLen == EqualCount(Suffix(), other.Suffix(), m_nSuffixLen));

    if (!other.m_mapChilds.IsEmpty())
    {
        if (m_mapChilds.IsEmpty())
        {
            m_mapChilds = other.m_mapChilds;
        }
        else
        {
            m_mapChilds.Merge(other.m_mapChilds, a.aMap, a);
        }
    }        
    
    a.aMap.aItem.Free(&other);    
}

//---------------------------------------------------------------------------

template class Node<false, false, titExact >; 
template class Node<false, true , titExact >; 
template class Node<true,  false, titExact >; 
template class Node<true,  true , titExact >; 
template class Node<false, false, titPrefix>; 
template class Node<false, true , titPrefix>; 
template class Node<true,  false, titPrefix>; 
template class Node<true,  true , titPrefix>; 
template class Node<false, false, titInfix >; 
template class Node<false, true , titInfix >; 
template class Node<true,  false, titInfix >; 
template class Node<true,  true , titInfix >; 

}//namespace Build

}//namespace FastFish

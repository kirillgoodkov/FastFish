#pragma once
#include "Containers/CharMapSmall.h"
#include "Containers/Deque.h"
#include "Containers/Set2.h"
#include "Memory/AllocatorInvaderBB.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

template<bool bCTRL, bool bSENS, TextIndexType tit>
class Node
{
    friend class ReorderBFS<bCTRL, bSENS, tit>;
    
    Node(const Node&);
    Node& operator = (const Node&);    
public:
    typedef Set2<docid_t, MaxDocumentId>      SetType;//@@@
    typedef CharMapSmall<Node, bCTRL, bSENS> MapType;

    struct ContextAW
    {
        docid_t         did;
        TextAllocator*  pA;
        const char*     pWord;
        wordsize_t      nWordLen;
        int4_t          nSkipLen;
    };

    Node() throw() : m_nSuffixLen(0) {};
    
    void            AddWord(ContextAW& ctx)                 throw();
    void            Merge(Node& other, TextAllocator& a)    throw();

    const char*     Suffix()    const throw();
    wordsize_t      SuffixLen() const throw()   {return m_nSuffixLen;}
    const SetType&  Postings()  const throw()   {return m_setPostings;}
    const MapType&  Childs()    const throw()   {return m_mapChilds;}
    
private:
    typedef CharMapSmallPd<Node, bCTRL, bSENS> MapTypePd;
    
    void Split(wordsize_t nFrom, TextAllocator& a) throw();

    char* Suffix() throw();

    union
    {
        char*       m_pSuffix;
        char        m_arrSuffix[PC::NodeInPlaceSuffixLen];
    };
    SetType         m_setPostings;
    wordsize_t      m_nSuffixLen;
    MapType         m_mapChilds;
    
};

}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/bNode.inl"

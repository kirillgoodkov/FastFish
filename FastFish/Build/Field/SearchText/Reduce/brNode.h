#pragma once
#include "Containers/CharMapFast.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{
namespace Reduce{

template<bool bCTRL, bool bSENS>
class Node
{
    Node(const Node&);
    Node& operator = (const Node&);
public:
    typedef CharMapFast<Node, bCTRL, bSENS> MapType;
    
    Node() throw();    
    void AddWord(const char* pWord, wordsize_t nLen, AllocatorInvader& a) throw();    
    
    static bool Find(const MapType& mapRoot, const char* pWord, wordsize_t nLen) throw();

    const char*     Suffix()    const throw()   {return m_pSuffix;}
    wordsize_t      SuffixLen() const throw()   {return m_nSuffixLen;}
    const MapType&  Childs()    const throw()   {return m_mapChilds;}      
    
private:    
    void Split(wordsize_t nFrom, AllocatorInvader& a) throw();
    
    typedef CharMapFastPd<Node, bCTRL, bSENS> MapTypePd;

    const char*     m_pSuffix;

    wordsize_t      m_nSuffixLen;
    MapType         m_mapChilds;    
};

}//namespace Reduce
}//namespace Build
}//namespace FastFish
#include "Build/Field/SearchText/Reduce/brNode.inl"

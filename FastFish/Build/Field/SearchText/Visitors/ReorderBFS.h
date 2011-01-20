#pragma once
#include "Containers/CharMapSmall.h"
#include "Memory/ArrayPtr.h"
#include "Common/Common.h"

namespace FastFish{
namespace Build{

//BFS - Breadth-first search
template<bool bCTRL, bool bSENS, TextIndexType tit>
class ReorderBFS
{
    ReorderBFS(const ReorderBFS&);
    ReorderBFS& operator = (const ReorderBFS&);
public:
    typedef Node<bCTRL, bSENS, tit> NodeType;
    typedef CharMapSmall<NodeType, bCTRL, bSENS> MapType;
    
    ReorderBFS(size_t nCount)                   throw();     
    void Reorder(MapType& mapRoot)              throw(); 
    void operator()(char ch, NodeType*& pNode)  throw();    

    const NodeType& At(size_t n)                                    const throw();    
    void StoreCharsFrom(OFStream& file, size_t nPos, size_t nCount) const ffThrowAll;

    const size_t        c_nCount;        
private:        
    
    ArrayPtr<NodeType*> m_arrNodes;
    ArrayPtr<char>      m_arrChars;
    
    size_t              m_nNextPos;        
};

}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/Visitors/ReorderBFS.inl"

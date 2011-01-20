#pragma once
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{

#pragma pack(push, 1)
template<typename VALUE, size_t nMAXVAL>
class Set
{
    Set(const Set&);
public:
    Set()                                               throw()     : m_nState(StateStoreCount) {}
    void CopyFrom(const Set& src, AllocatorInvader& a)  throw();
    Set& operator = (Set& oth)                          throw();
    
    void Insert(VALUE val, AllocatorInvader& a)         throw();
    void Merge(Set& other, AllocatorInvader& a)         throw();

    VALUE Count()                                 const throw();
    bool  IsExist(VALUE n)                        const throw();
    bool  IsEmpty()                               const throw()     {return StateStoreCount == m_nState;}
    bool  HasOneItem()                            const throw();
    
    template<typename PROC>
    void Enum(PROC& proc)                         const ffThrowAll;
    
private:
    template<typename PROC>
    static void Enum(PROC& proc, const VALUE* pVal) ffThrowAll;

    void PrepareForTree(AllocatorInvader& a) throw();

#ifdef ffDebug    
    bool Check() const throw();    
    static size_t ValuesCount(VALUE* pLeaf) throw();
#endif    

    static const size_t LeafItemCount      = (PC::SetBlockSize - sizeof(void*))/sizeof(VALUE);
    static const size_t NodeItemCount      = PC::SetBlockSize/sizeof(void*) - 1;
    static const size_t InPlaceItemCount   = ff32Or64(3, sizeof(VALUE*)/sizeof(VALUE));                                            
    static const VALUE  StateStoreCount    = nMAXVAL + 1;    
    static const size_t BlockIntMask       = PC::SetBlockSize - 1; 
    static const size_t BlockExtMask       = ~BlockIntMask;
    
    struct Leaf
    {
        VALUE*  pPrev;
        VALUE   arrVals[LeafItemCount];
    };
    
    struct Node
    {
        Node*   pPrev;
        VALUE*  arrLeafs[NodeItemCount];
    };
    
    static Leaf* GetLeaf(VALUE* pVal)               throw();
    static Node* GetNode(VALUE** ppVal)             throw();
    static const Leaf* GetLeaf(const VALUE* pVal)   throw() {return GetLeaf(const_cast<VALUE*>(pVal));}
    static const Node* GetNode(VALUE*const* ppVal)  throw() {return GetNode(const_cast<VALUE**>(ppVal));}
    
    union 
    {
        VALUE   m_arrVals[InPlaceItemCount];
        VALUE*  m_pLeaf;
        VALUE** m_pNode;
    };
    
    VALUE       m_nState;
};
#pragma pack(pop)

    


}//namespace FastFish
#include "Containers/Set.inl"

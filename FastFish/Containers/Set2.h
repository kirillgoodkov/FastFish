#pragma once
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{

#pragma pack(push, 1)

template<typename VALUE, size_t nMAXVAL>
class Set2
{
    Set2(const Set2&);
public:
    Set2()                                              throw();
    void CopyFrom(const Set2& src, AllocatorInvader& a) throw();
    Set2& operator = (Set2& other)                      throw();
    
    void Insert(VALUE val, AllocatorInvader& a)         throw();
    void Merge(Set2& other, AllocatorInvader& a)        throw();

    VALUE Count()                                 const throw();
    bool  IsExist(VALUE val)                      const throw();
    bool  IsEmpty()                               const throw()                 {return ValueNop == m_lst.valLast;}

    bool  HasOneItem()                            const throw();
    
    template<typename PROC>
    void Enum(PROC& proc)                         const ffThrowAll;

private:
    static const size_t BlockShift  = 6;
    static const size_t BlockSize   = 1 << BlockShift;
    static const size_t BlockMask   = BlockSize - 1;

    static const size_t InplaceVals = ff32Or64(3, 5);
    static const size_t DataSize    = BlockSize - sizeof(void*);
    static const size_t ChainsCount = DataSize / sizeof(void*);
    
    static const VALUE  ValueNop    = nMAXVAL + 1;
    static const VALUE  ValueFlag   = ~(VALUE(-1) >> 1);
    
    struct Leaf
    {
        uns1_t      arrData[DataSize];      //fill in reverse order, 0 <= items <= DataSize
        uns1_t*     pPrev;
        
        uns1_t*         DataEnd() throw()       {return arrData + ffCountOf(arrData);}
        const uns1_t*   DataEnd() const throw() {return arrData + ffCountOf(arrData);}
    };      
    struct Node
    {
        uns1_t*     arrChains[ChainsCount]; //fill in normal order, 0 < items <= LeafsCount
        uns1_t**    pPrev;
        
        uns1_t**    ChainsLast() throw()        {return arrChains + ffCountOf(arrChains) - 1;}
    };      
    struct List
    {
        uns1_t*     pWrite;
        ff64Only(VALUE pad);
        VALUE       nCountF;
        VALUE       valLast;
    };  
    struct Tree
    {
        uns1_t**    pWrite;
        ff64Only(VALUE pad);
        VALUE       nCountF;
        VALUE       valLastF;
    };
    struct Raw
    {
        void*   p[2];
        VALUE   v;        
    };
    
    union
    {
        VALUE       m_arrVals[InplaceVals];
        List        m_lst;
        Tree        m_tree;
        Raw         m_raw;
    };

    static Leaf* GetLeaf(uns1_t* p)                    throw() {return reinterpret_cast<Leaf*>(size_t(p) & ~BlockMask);}
    static const Leaf* GetLeaf(const uns1_t* p)        throw() {return reinterpret_cast<Leaf*>(size_t(p) & ~BlockMask);}
    
    static Node* GetNode(uns1_t** pp)                  throw() {return reinterpret_cast<Node*>(size_t(pp) & ~BlockMask);}
    static const Node* GetNode(const uns1_t*const* pp) throw() {return reinterpret_cast<Node*>(size_t(pp) & ~BlockMask);}

    static Leaf* AppendLeaf(uns1_t*& pDst, uns1_t* pSrc, AllocatorInvader& a)               throw();
    static Leaf* Insert2Leaf(uns1_t*& pDst, Leaf* pLeaf, VALUE val, AllocatorInvader& a)    throw();
    static bool  IsLeafFull(const uns1_t* p)                                                throw()     {return GetLeaf(p)->arrData == p;}
    
    VALUE* ValsEnd()              throw() {return m_arrVals + ffCountOf(m_arrVals);}    
    const VALUE* ValsEnd()  const throw() {return m_arrVals + ffCountOf(m_arrVals);}
                            
    VALUE* ValsLast()             throw() {return m_arrVals + ffCountOf(m_arrVals) - 1;}    
    const VALUE* ValsLast() const throw() {return m_arrVals + ffCountOf(m_arrVals) - 1;}
                            
    bool IsInplace()        const throw() {return 0 == (m_lst.nCountF & ValueFlag);}
    bool IsTree()           const throw() {return 0 != (m_tree.valLastF & ValueFlag);}
                            
    void Clear()                  throw() {std::fill_n(m_arrVals, ffCountOf(m_arrVals), VALUE(ValueNop));}

    template<typename PROC>
    static void EnumChain(PROC& proc, VALUE valPrev, const uns1_t* pRead) ffThrowAll;
    
};
#pragma pack(pop)

}//namespace FastFish
#include "Containers/Set2.inl"

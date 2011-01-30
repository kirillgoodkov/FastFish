#include "Memory/AllocatorInvader.h"
#include "Tools/Bits.h"

namespace FastFish{

template<typename VALUE, size_t nMAXVAL>
typename Set<VALUE, nMAXVAL>::Leaf* Set<VALUE, nMAXVAL>::AppendLeaf(uns1_t*& pDst, uns1_t* pSrc, AllocatorInvader& a) throw()
{
    ffAssume(pSrc);
    Leaf* pLeaf  = NewPOD<Leaf>(a);
    pLeaf->pPrev = pSrc;                    
    pDst         = pLeaf->DataEnd();
    return pLeaf;
}

template<typename VALUE, size_t nMAXVAL>
typename Set<VALUE, nMAXVAL>::Leaf* Set<VALUE, nMAXVAL>::Insert2Leaf(uns1_t*& pDst, Leaf* pLeaf, VALUE val, AllocatorInvader& a) throw()
{
    ffAssume(pLeaf);
    ffAssume(pLeaf->arrData <= pDst && pDst <= pLeaf->arrData + ffCountOf(pLeaf->arrData));
    if (size_t(pDst - pLeaf->arrData) < VbeSizeOf(val))
    {
        ffClFlush(pLeaf);
        pLeaf = AppendLeaf(pDst, pDst, a);
    }
    VbeWriteRev(val, pDst);    
    return pLeaf;
}

//----------------------------------------------------------------------------

template<typename VALUE, size_t nMAXVAL>
bool Set<VALUE, nMAXVAL>::IsExist(VALUE val) const throw()
{
    ffAssert(!IsTree());    
    return val == m_lst.valLast;
}

template<typename VALUE, size_t nMAXVAL>
bool Set<VALUE, nMAXVAL>::HasOneItem() const throw()
{
    ffAssert(!IsTree());    
    return ValueNop != *ValsLast() && ValueNop == *(ValsLast() - 1);
}                       

template<typename VALUE, size_t nMAXVAL>
VALUE Set<VALUE, nMAXVAL>::Count() const throw()
{
    if (IsInplace())
    {
        const VALUE* pVal = ValsLast();
        for (; m_arrVals <= pVal && *pVal != ValueNop; --pVal)
        {}    
        return VALUE(ValsLast() - pVal);    
    }
    else
    {
        return m_lst.nCountF & ~ValueFlag;
    }
}

//----------------------------------------------------------------------------

template<typename VALUE, size_t nMAXVAL>
Set<VALUE, nMAXVAL>::Set() throw()
{
    Clear();
    ffAssumeStatic(sizeof(*this) == sizeof(List));
    ffAssumeStatic(sizeof(*this) == sizeof(Tree));
    ffAssumeStatic(sizeof(*this) == sizeof(m_arrVals));
    ffAssumeStatic(sizeof(*this) == sizeof(Raw));
    ffAssumeStatic(nMAXVAL <= (VALUE(-1) >> 1));
    ffAssumeStatic(sizeof(VALUE) <= sizeof(void*));
}

template<typename VALUE, size_t nMAXVAL>
void Set<VALUE, nMAXVAL>::CopyFrom(const Set& src, AllocatorInvader& a) throw()
{
    ffAssert(IsEmpty());
    if (src.IsInplace())
    {
        m_raw = src.m_raw;
    }
    else if (!src.IsTree())//list
    {
        if (IsLeafFull(src.m_lst.pWrite))
        {
            m_lst.pWrite = src.m_lst.pWrite;
        }
        else
        {
            AppendLeaf(m_lst.pWrite, src.m_lst.pWrite, a);
        }
        m_lst.nCountF = src.m_lst.nCountF;
        m_lst.valLast = src.m_lst.valLast;
    }
    else//tree
    {
        Node* pNodeSrc      = GetNode(src.m_tree.pWrite);
        Node* pNode         = NewPOD<Node>(a);
        pNode->pPrev        = (pNodeSrc->arrChains == src.m_tree.pWrite) ? pNodeSrc->pPrev : (src.m_tree.pWrite - 1);
        pNode->arrChains[0] = *src.m_tree.pWrite;

        m_tree.pWrite   = pNode->arrChains;
        m_tree.nCountF  = src.m_tree.nCountF;
        m_tree.valLastF = src.m_tree.valLastF;                        

        if (!IsLeafFull(*m_tree.pWrite))
        {
            AppendLeaf(*m_tree.pWrite, *m_tree.pWrite, a);
        }        
    }
    
    ffAssert(Check());
}

template<typename VALUE, size_t nMAXVAL>
Set<VALUE, nMAXVAL>& Set<VALUE, nMAXVAL>::operator = (Set& other) throw()
{
    ffAssert(IsEmpty());
    ffAssert(other.Check());
    m_raw = other.m_raw;
    other.Clear();
    return *this;
}

template<typename VALUE, size_t nMAXVAL> ffForceInline
void Set<VALUE, nMAXVAL>::Insert(VALUE val, AllocatorInvader& a) throw()
{
    ffAssert(!IsTree());
    ffAssert(!IsExist(val));
    
    if (IsInplace())
    {
        if (m_arrVals[0] == ValueNop)
        {//inplace
            std::copy(m_arrVals + 1, ValsEnd(), m_arrVals);
            *ValsLast() = val;
        }
        else
        {//inplace -> list
            ffAssumeStatic(InplaceVals <= DataSize/VbeMaxLen);
            Leaf* pLeaf  = NewPOD<Leaf>(a);
            pLeaf->pPrev = 0;
            
            VALUE valPrev  = m_arrVals[0];
            uns1_t* pWrite = pLeaf->DataEnd();
            for (VALUE* pVal = m_arrVals + 1; pVal != ValsEnd(); ++pVal)
            {
                VbeWriteRev(*pVal - valPrev, pWrite);
                valPrev = *pVal;
            }
            
            VbeWriteRev(val - valPrev, pWrite);            
            m_lst.pWrite  = pWrite;
            m_lst.nCountF = VALUE(InplaceVals + 1) | ValueFlag;            
            m_lst.valLast = val;
        }                        
    }
    else
    {//list
        ffAssert(InplaceVals < Count() && Count() < nMAXVAL);
        Insert2Leaf(m_lst.pWrite, GetLeaf(m_lst.pWrite), val - m_lst.valLast, a);
        ++m_lst.nCountF;        
        m_lst.valLast = val;
    }
    
    ffDeepCheckOnly(ffAssert(Check()));
    ffAssert(IsExist(val));
}

template<typename VALUE, size_t nMAXVAL>
template<typename PROC>
void Set<VALUE, nMAXVAL>::EnumChain(PROC& proc, VALUE valPrev, const uns1_t* pRead) ffThrowAll
{
    proc(valPrev);    
    do
    {
        const Leaf* pLeaf = GetLeaf(pRead);                            
        while (pRead != pLeaf->DataEnd())
        {
            valPrev -= VbeRead(pRead);
            proc(valPrev);
        }        
        pRead = pLeaf->pPrev;
    }            
    while (pRead);      
}        

template<typename VALUE, size_t nMAXVAL>
template<typename PROC>
void Set<VALUE, nMAXVAL>::Enum(PROC& proc) const ffThrowAll
{
    if (IsInplace())
    {
        for (const VALUE* pVal = ValsLast(); m_arrVals <= pVal && *pVal != ValueNop; --pVal)
        {
            proc(*pVal);
        }    
    }
    else if (!IsTree())//list
    {
        EnumChain(proc, m_lst.valLast, m_lst.pWrite);
    }
    else //tree
    {    
        EnumChain(proc, m_tree.valLastF & ~ValueFlag, *m_tree.pWrite);
        
        const uns1_t*const* pRead = m_tree.pWrite - 1;
        do //nodes
        {
            const Node* pNode = GetNode(pRead + 1);
            for (; pNode->arrChains <= pRead; --pRead)
            {
                const uns1_t* pData = *pRead;
                VALUE valPrev = VbeRead(pData);
                EnumChain(proc, valPrev, pData);
            }                        
            pRead = pNode->pPrev;
        }
        while (pRead);
    }
}

template<typename VALUE, size_t nMAXVAL> ffForceInline
void Set<VALUE, nMAXVAL>::Merge(Set& other, AllocatorInvader& a) throw()
{
    //      (this)      (other)
    //0     inplace     inplace     ->  inplace                 ? sum <= inplace
    //                              |   convert2list, goto 1                                
    //1     list        inplace     ->  list                      
    //2     list        list        ->  convert2tree, goto 4                                
    //3     tree        inplace     ->  tree(same chain)                                        
    //4     tree        list        ->  tree(new chain)

    ffAssert(!other.IsTree());
    if (IsEmpty())
    {
        m_raw = other.m_raw;
        return;
    }
    
    if (other.IsEmpty())
    {
        return;
    }        

    ffAssert(Check() && other.Check());   

    VALUE nCountOther = other.Count();

    if (IsInplace())
    {
        VALUE nCount = Count();
        if (other.IsInplace() && nCount + nCountOther <= InplaceVals)
        {
            std::copy(ValsEnd() - nCount, ValsEnd(), ValsEnd() - nCount - nCountOther);
            std::copy(other.ValsEnd() - nCountOther, other.ValsEnd(), ValsEnd() - nCountOther);
            ffDebugOnly(other.Clear());
            return;
        }
        
        //convert this 2 list
        ffAssumeStatic(InplaceVals <= DataSize/VbeMaxLen);
        Leaf* pLeaf  = NewPOD<Leaf>(a);
        pLeaf->pPrev = 0;
        
        VALUE valPrev  = *(ValsEnd() - nCount);
        uns1_t* pWrite = pLeaf->DataEnd();
        for (VALUE* pVal = ValsEnd() - nCount + 1; pVal != ValsEnd(); ++pVal)
        {
            VbeWriteRev(*pVal - valPrev, pWrite);
            valPrev = *pVal;
        }
        
        m_lst.pWrite  = pWrite;
        m_lst.nCountF = nCount | ValueFlag;
        ffAssert(m_lst.valLast == *ValsLast());
    }
    
    if (!IsTree())//list
    {
        if (other.IsInplace())
        {
            Leaf* pLeaf   = GetLeaf(m_lst.pWrite);            
            VALUE valPrev = m_lst.valLast;
            for (VALUE* pVal = other.ValsEnd() - nCountOther; pVal != other.ValsEnd(); ++pVal)
            {
                pLeaf   = Insert2Leaf(m_lst.pWrite, pLeaf, *pVal - valPrev, a);
                valPrev = *pVal;
            }
            
            m_lst.nCountF += nCountOther;
            m_lst.valLast  = valPrev;        
            ffDebugOnly(other.Clear());               
            return;
        }
        else//other.list
        {//convert this 2 tree
            ffAssert(!(IsInplace() || IsTree()));
            
            Node* pNode         = NewPOD<Node>(a);
            pNode->pPrev        = 0;
            pNode->arrChains[0] = m_lst.pWrite;
            m_tree.pWrite       = pNode->arrChains;
            m_tree.valLastF    |= ValueFlag;        
        }
    }
    
    ffAssert(IsTree());    
    if (other.IsInplace())
    {
        //push 2 same leaf chain
        Leaf* pLeaf   = GetLeaf(*m_tree.pWrite);
        VALUE valPrev = m_tree.valLastF & ~ValueFlag;
        for (VALUE* pVal = other.ValsEnd() - nCountOther; pVal != other.ValsEnd(); ++pVal)
        {
            pLeaf   = Insert2Leaf(*m_tree.pWrite, pLeaf, *pVal - valPrev, a);
            valPrev = *pVal;
        }
        
        m_tree.nCountF += nCountOther;                
        m_tree.valLastF = valPrev | ValueFlag;                
    }
    else
    {   //new leaf chain
        Leaf* pLeaf = GetLeaf(*m_tree.pWrite);
        Insert2Leaf(*m_tree.pWrite, pLeaf, m_tree.valLastF & ~ValueFlag, a);
        ffClFlush(pLeaf);
        
        Node* pNode = GetNode(m_tree.pWrite);
        if (pNode->ChainsLast() == m_tree.pWrite)
        {
            ffClFlush(pNode);
            pNode         = NewPOD<Node>(a);
            pNode->pPrev  = m_tree.pWrite;
            m_tree.pWrite = pNode->arrChains;
        }
        else
        {
            ++m_tree.pWrite;
        }
        
        *m_tree.pWrite  = other.m_lst.pWrite;
        m_tree.nCountF += other.m_lst.nCountF & ~ValueFlag;
        m_tree.valLastF = other.m_lst.valLast | ValueFlag;         
    }
    ffAssert(IsTree() && InplaceVals < Count());
    
    ffDebugOnly(other.Clear());                   
}

//----------------------------------------------------------------------------

#ifdef ffDebug

template<typename VALUE, size_t nMAXVAL>
struct Counter
{
    size_t n;
    void operator() (VALUE val) throw() {++n; ffAssume(val <= nMAXVAL);}        
};

template<typename VALUE, size_t nMAXVAL>
bool Set<VALUE, nMAXVAL>::Check() const throw()
{
    Counter<VALUE, nMAXVAL> cnt = {0};
    Enum(cnt);
    return Count() == cnt.n;
}

#endif

}//namespace FastFish

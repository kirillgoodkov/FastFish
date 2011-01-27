#include "Memory/AllocatorInvader.h"
#include "Tools/Bits.h"

namespace FastFish{

template<typename VALUE, size_t nMAXVAL>
bool Set2<VALUE, nMAXVAL>::IsExist(VALUE val) const throw()
{
    ffAssert(!IsTree());    
    return val == m_lst.valLast;
}

template<typename VALUE, size_t nMAXVAL>
bool Set2<VALUE, nMAXVAL>::HasOneItem() const throw()
{
    ffAssert(!IsTree());    
    return ValueNop != *ValsLast() && 
           ValueNop == *(ValsLast() - 1);
}                       

template<typename VALUE, size_t nMAXVAL>
VALUE Set2<VALUE, nMAXVAL>::Count() const throw()
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

/*
template<typename VALUE, size_t nMAXVAL>
bool Set2<VALUE, nMAXVAL>::HasSingleLeaf() const throw() 
{
    ffAssert(!(IsInplace() || IsTree()));
    Leaf* pLeaf = GetLeaf(m_lst.pWrite);
    return 0 == pLeaf->pPrev &&            //leaf is last
           pLeaf->arrData != m_lst.pWrite; //not full, because full leaf may be shared
}
*/

//----------------------------------------------------------------------------

template<typename VALUE, size_t nMAXVAL>
Set2<VALUE, nMAXVAL>::Set2() throw()
{
    Clear();
    ffAssumeStatic(sizeof(*this) == sizeof(List));
    ffAssumeStatic(sizeof(*this) == sizeof(Tree));
    ffAssumeStatic(sizeof(*this) == sizeof(m_arrVals));
    ffAssumeStatic(sizeof(*this) == sizeof(Raw));
}

template<typename VALUE, size_t nMAXVAL>
void Set2<VALUE, nMAXVAL>::CopyFrom(const Set2& src, AllocatorInvader& a)throw()
{
    ffAssert(IsEmpty());
    if (src.IsInplace())
    {
        m_raw = src.m_raw;
    }
    else if (!src.IsTree())//list
    {
        Leaf* pLeaf = GetLeaf(src.m_lst.pWrite);
        if (src.m_lst.pWrite == pLeaf->arrData)
        {//full leaf, attach directly
            m_lst.pWrite = src.m_lst.pWrite;
        }
        else
        {
            Leaf* pLeaf  = NewPOD<Leaf>(a);
            pLeaf->pPrev = src.m_lst.pWrite;                    
            m_lst.pWrite = pLeaf->DataEnd();                                        
        }
        m_lst.nCountF = src.m_lst.nCountF;
        m_lst.valLast = src.m_lst.valLast;
    }
    else//tree
    {
        Node* pNode     = NewPOD<Node>(a);
        Node* pNodeSrc  = GetNode(src.m_tree.pWrite);
        *pNode          = *pNodeSrc;
        
        m_tree.pWrite   = pNode->arrLeafs + (src.m_tree.pWrite - pNodeSrc->arrLeafs);
        m_tree.nCountF  = src.m_tree.nCountF;
        m_tree.valLastF = src.m_tree.valLastF;                
        
        Leaf* pLeaf = GetLeaf(*m_tree.pWrite);
        if (pLeaf->arrData != *m_tree.pWrite)
        {
            pLeaf          = NewPOD<Leaf>(a);
            pLeaf->pPrev   = *m_tree.pWrite;
            *m_tree.pWrite = pLeaf->DataEnd();
        }        
    }
}

template<typename VALUE, size_t nMAXVAL>
Set2<VALUE, nMAXVAL>& Set2<VALUE, nMAXVAL>::operator = (Set2& other) throw()
{
    m_raw = other.m_raw;
    ffDebugOnly(other.Clear());
    return *this;
}

template<typename VALUE, size_t nMAXVAL>
void Set2<VALUE, nMAXVAL>::Insert(VALUE val, AllocatorInvader& a) throw()
{
    ffAssert(!IsTree());
    ffAssert(!IsExist(val));
    
    if (IsInplace())
    {
        if (m_arrVals[0] == ValueNop)
        {//inplace
            for (VALUE* pVal = m_arrVals; pVal != ValsLast(); ++pVal)
            {
                *pVal = *(pVal + 1);
            }
            m_arrVals[InplaceVals - 1] = val;
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
            m_lst.valLast = val;
            m_lst.nCountF = VALUE(InplaceVals + 1) | ValueFlag;            
        }                        
    }
    else
    {//list
        Leaf* pLeaf = GetLeaf(m_lst.pWrite);
        VALUE valDiff = val - m_lst.valLast;

        ffAssume(0 <= m_lst.pWrite - pLeaf->arrData);
        if (size_t(m_lst.pWrite - pLeaf->arrData) < VbeSizeOf(valDiff))
        {//new leaf
            pLeaf        = NewPOD<Leaf>(a);
            pLeaf->pPrev = m_lst.pWrite;
            m_lst.pWrite = pLeaf->DataEnd();
        }
        
        VbeWriteRev(valDiff, m_lst.pWrite);
        m_lst.valLast = val;
        ++m_lst.nCountF;
    }
}

template<typename VALUE, size_t nMAXVAL>
template<typename PROC>
void Set2<VALUE, nMAXVAL>::EnumChain(PROC& proc, VALUE valPrev, const uns1_t* pRead) ffThrowAll
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
void Set2<VALUE, nMAXVAL>::Enum(PROC& proc) const ffThrowAll
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
            for (; pNode->arrLeafs <= pRead; --pRead)
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

/*          this        other
    0       inplace     inplace     ->  inplace                 ? sum <= inplace
                                    |   convert 2 list, goto 1                                
    1       list        inplace     ->  list                      
    2       list        list        ->  convert 2 tree, goto 4                                
    3       tree        inplace     ->  tree(same pos)                                        
    4       tree        list        ->  tree(new pos)
*/

template<typename VALUE, size_t nMAXVAL>
void Set2<VALUE, nMAXVAL>::Merge(Set2& other, AllocatorInvader& a) throw()
{
    ffAssert(!other.IsTree());
    ffAssert(!(IsEmpty() || other.IsEmpty()));

    VALUE nCountOther = other.Count();

    if (IsInplace())
    {
        VALUE nCount = Count();
        if (other.IsInplace() && 
            nCount + nCountOther <= InplaceVals)
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
        
        VALUE* pVal    = ValsEnd() - nCount;
        VALUE valPrev  = *pVal++;
        uns1_t* pWrite = pLeaf->DataEnd();
        for (; pVal != ValsEnd(); ++pVal)
        {
            VbeWriteRev(*pVal - valPrev, pWrite);
            valPrev = *pVal;
        }
        m_lst.pWrite  = pWrite;
        m_lst.nCountF = nCount | ValueFlag;
    }
    
    if (!IsTree())//list
    {
        Leaf* pLeaf = GetLeaf(m_lst.pWrite);
        if (other.IsInplace())
        {
            VALUE valPrev = m_lst.valLast;
            for (VALUE* pVal = other.ValsEnd() - nCountOther; pVal != other.ValsEnd(); ++pVal)
            {
                VALUE valDiff = *pVal - valPrev;
                ffAssume(0 <= m_lst.pWrite - pLeaf->arrData);
                if (size_t(m_lst.pWrite - pLeaf->arrData) < VbeSizeOf(valDiff))
                {//new leaf
                    pLeaf        = NewPOD<Leaf>(a);
                    pLeaf->pPrev = m_lst.pWrite;
                    m_lst.pWrite = pLeaf->DataEnd();
                }
                
                VbeWriteRev(valDiff, m_lst.pWrite);
                valPrev = *pVal;
            }
            
            m_lst.valLast  = valPrev;        
            m_lst.nCountF += nCountOther;
            ffDebugOnly(other.Clear());               
            return;
        }
        else//other.list
        {//convert this 2 tree
            ffAssert(!(IsInplace() || IsTree()));
            
            Node* pNode        = NewPOD<Node>(a);
            pNode->pPrev       = 0;
            pNode->arrLeafs[0] = m_lst.pWrite;
            m_tree.pWrite      = pNode->arrLeafs;
            m_tree.valLastF    = m_lst.valLast | ValueFlag;        
        }
    }
    
    ffAssert(IsTree());

    Leaf* pLeaf = GetLeaf(*m_tree.pWrite);
    ffAssume(0 <= *m_tree.pWrite - pLeaf->arrData);
    
    if (other.IsInplace())
    {
        //push 2 same leaf chain
        VALUE valPrev = m_tree.valLastF & ~ValueFlag;
        for (VALUE* pVal = other.ValsEnd() - nCountOther; pVal != other.ValsEnd(); ++pVal)
        {
            VALUE valDiff = *pVal - valPrev;
            if (size_t(*m_tree.pWrite - pLeaf->arrData) < VbeSizeOf(valDiff))
            {//new leaf
                pLeaf          = NewPOD<Leaf>(a);
                pLeaf->pPrev   = *m_tree.pWrite;
                *m_tree.pWrite = pLeaf->DataEnd();
            }            
            VbeWriteRev(valDiff, *m_tree.pWrite);
            valPrev = *pVal;
        }
        
        m_tree.valLastF = valPrev | ValueFlag;                
        m_tree.nCountF += nCountOther;                
    }
    else
    {   //new leaf chain
        VALUE valLast = m_tree.valLastF & ~ValueFlag;
        if (size_t(*m_tree.pWrite - pLeaf->arrData) < VbeSizeOf(valLast))
        {//new leaf
            pLeaf          = NewPOD<Leaf>(a);
            pLeaf->pPrev   = *m_tree.pWrite;
            *m_tree.pWrite = pLeaf->DataEnd();
        }        
        VbeWriteRev(valLast, *m_tree.pWrite);
        
        Node* pNode = GetNode(m_tree.pWrite);
        if (pNode->LeafsLast() == m_tree.pWrite)
        {//new node
            pNode         = NewPOD<Node>(a);
            pNode->pPrev  = m_tree.pWrite;
            m_tree.pWrite = pNode->arrLeafs;
        }
        else
        {
            ++m_tree.pWrite;
        }
        
        *m_tree.pWrite  = other.m_lst.pWrite;
        m_tree.nCountF += other.m_lst.nCountF & ~ValueFlag;
        m_tree.valLastF = other.m_lst.valLast | ValueFlag;         
    }
    
    ffDebugOnly(other.Clear());                   
}

}//namespace FastFish

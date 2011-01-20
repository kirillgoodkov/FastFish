#include "Memory/AllocatorInvader.h"
namespace FastFish{

template<typename VALUE, size_t nMAXVAL> 
typename Set<VALUE, nMAXVAL>::Leaf* Set<VALUE, nMAXVAL>::GetLeaf(VALUE* pVal) throw()
{
    ffDebugOnly(size_t nDiff = size_t(pVal) - (size_t(pVal - 1) & BlockExtMask));
    ffAssert(sizeof(void*) <= nDiff && nDiff <= PC::SetBlockSize);
    return reinterpret_cast<Leaf*>(size_t(pVal - 1) & BlockExtMask);
}

template<typename VALUE, size_t nMAXVAL> 
typename Set<VALUE, nMAXVAL>::Node* Set<VALUE, nMAXVAL>::GetNode(VALUE** pVal) throw()
{
    return reinterpret_cast<Node*>(size_t(pVal) & BlockExtMask);
}

//---------------------------------------------------------------------------

template<typename VALUE, size_t nMAXVAL>
void Set<VALUE, nMAXVAL>::CopyFrom(const Set& src, AllocatorInvader& a) throw()
{
    ffAssert(src.Check());
    ffAssume(StateStoreCount == m_nState);
    
    if (src.m_nState < StateStoreCount)
    {//list
        const Leaf* pSrcLeaf = GetLeaf(src.m_pLeaf);
        size_t nValsCount    = src.m_pLeaf - pSrcLeaf->arrVals;
        
        Leaf* pLeaf          = static_cast<Leaf*>(a.Alloc());    
        memcpy(pLeaf, pSrcLeaf, sizeof(VALUE*) + sizeof(VALUE) * (nValsCount + 1));
        
        m_nState = src.m_nState;
        m_pLeaf  = pLeaf->arrVals + nValsCount;
    }
    else
    {
        VALUE nCount = src.m_nState - StateStoreCount;
        if (InPlaceItemCount < nCount)
        {//tree
            Node* pNode = static_cast<Node*>(a.Alloc());
            Leaf* pLeaf = static_cast<Leaf*>(a.Alloc());
            
            const Node* pNodeSrc = GetNode(src.m_pNode);
            size_t nLeafsCount   = src.m_pNode - pNodeSrc->arrLeafs + 1;
            memcpy(pNode, pNodeSrc, sizeof(Node*) + sizeof(VALUE*) * (nLeafsCount - 1));

            const Leaf* pSrcLeaf = GetLeaf(*src.m_pNode);
            size_t nValsCount    = *src.m_pNode - pSrcLeaf->arrVals;
            memcpy(pLeaf, pSrcLeaf, sizeof(VALUE*) + sizeof(VALUE) * nValsCount);

            m_pNode  = pNode->arrLeafs + nLeafsCount - 1;
            *m_pNode = pLeaf->arrVals + nValsCount;
            
            m_nState = src.m_nState;
        }
        else
        {//internal
            memcpy(this, &src, sizeof(*this));
        }
    }
    
    ffAssert(Check());
}

template<typename VALUE, size_t nMAXVAL> 
Set<VALUE, nMAXVAL>& Set<VALUE, nMAXVAL>::operator = (Set& oth) throw()
{
    memcpy(this, &oth, sizeof(*this));
    oth.m_nState = StateStoreCount;
    return *this;
}

template<typename VALUE, size_t nMAXVAL> ffForceInline
void Set<VALUE, nMAXVAL>::Insert(VALUE val, AllocatorInvader& a) throw()
{
    ffAssert(Check());
    ffAssert(!IsExist(val));
    
    if (m_nState < StateStoreCount)
    {//list
        Leaf* pLeafNew = (size_t(m_pLeaf + 1) & BlockIntMask) ? 
                                0:
                                static_cast<Leaf*>(a.Alloc());
        
        VALUE nCount = *m_pLeaf;
        *m_pLeaf++   = m_nState;
        m_nState     = val;
        
        if (pLeafNew)
        {
            pLeafNew->pPrev = m_pLeaf;
            m_pLeaf         = pLeafNew->arrVals;
        }
        
        *m_pLeaf = nCount + 1;            
    }
    else
    {
        VALUE nCount = m_nState - StateStoreCount;
        if (0 == nCount)
        {
            m_arrVals[0] = val;
            ++m_nState;            
        }
        else if (nCount < InPlaceItemCount)    
        {
            m_arrVals[nCount] = val;
            ++m_nState;
        }
        else if (InPlaceItemCount == nCount)    
        {
            Leaf* pLeaf = static_cast<Leaf*>(a.Alloc());
            pLeaf->pPrev = 0;
            for (size_t n = 0; n < nCount; ++n)
            {
                pLeaf->arrVals[n] = m_arrVals[n];
            }
            m_nState  = val;                
            m_pLeaf   = pLeaf->arrVals + nCount;
            *m_pLeaf  = nCount + 1;                
            ffAssume(0 != (size_t(m_pLeaf) & BlockIntMask));
        }        
        else
        {//tree
            Leaf* pLeafNew = (size_t(*m_pNode + 1) & BlockIntMask) ? 
                                0 : static_cast<Leaf*>(a.Alloc());

            Node* pNode    = GetNode(m_pNode);
            ffAssume(0 <= m_pNode - pNode->arrLeafs + 1);
            Node* pNodeNew = (pLeafNew && NodeItemCount == size_t(m_pNode - pNode->arrLeafs + 1)) ?
                                static_cast<Node*>(a.Alloc()) : 0;
              
            ++m_nState;
            **m_pNode = val;
            *m_pNode += 1;
            
            if (pLeafNew)
            {
                if (pNodeNew)
                {
                    pNodeNew->pPrev = pNode;
                    m_pNode         = pNodeNew->arrLeafs - 1;
                }
                
                pLeafNew->pPrev = 0;
                
                m_pNode += 1;
                *m_pNode = pLeafNew->arrVals;
            }
        }
    }

    ffAssert(IsExist(val));
    ffAssert(Check());
}

template<typename VALUE, size_t nMAXVAL>
void Set<VALUE, nMAXVAL>::PrepareForTree(AllocatorInvader& a) throw()
{
    ffAssume(m_nState < StateStoreCount);
    
    Leaf* pLeafNew = (size_t(m_pLeaf + 1) & BlockIntMask) ? 
                            0 : static_cast<Leaf*>(a.Alloc());
    
    VALUE nCount = *m_pLeaf;
    *m_pLeaf++   = m_nState;
    m_nState     = nCount;
    
    if (pLeafNew)
    {
        pLeafNew->pPrev = m_pLeaf;
        m_pLeaf         = pLeafNew->arrVals;
    }    
}

template<typename VALUE, size_t nMAXVAL> ffForceInline
void Set<VALUE, nMAXVAL>::Merge(Set& other, AllocatorInvader& a) throw()
{
    ffAssert(Check());
    ffAssert(other.Check());
    ffAssume(other.m_nState <= StateStoreCount + InPlaceItemCount);    
    
    if (StateStoreCount <= other.m_nState)
    {//other.internal
        VALUE nCount = other.m_nState - StateStoreCount;
        
        for (size_t n = 0; n < nCount; ++n)
        {
            Insert(other.m_arrVals[n], a);
        }                                         
    }
    else
    {//other.list
    
        if (StateStoreCount + InPlaceItemCount < m_nState)
        {//tree
            Node* pNode   = GetNode(m_pNode);
            size_t nLeafs = m_pNode - pNode->arrLeafs + 1;

            if (nLeafs == NodeItemCount)
            {
                Node* pNodeNew  = static_cast<Node*>(a.Alloc());
                other.PrepareForTree(a);
                pNodeNew->pPrev = pNode;
                m_pNode         = pNodeNew->arrLeafs - 1;
            }
            else
            {
                other.PrepareForTree(a);
            }
            
            m_pNode  += 1;
            *m_pNode  = other.m_pLeaf;
            m_nState += other.m_nState;
        }
        else
        {
            if (StateStoreCount == m_nState)
            {//empty
                memcpy(this, &other, sizeof(*this));
            }
            else
            {//list || internal
                Node* pNode = static_cast<Node*>(a.Alloc());
                
                if (StateStoreCount <= m_nState)
                {//internal
                    VALUE nCount = m_nState - StateStoreCount;
                    Leaf* pLeaf  = static_cast<Leaf*>(a.Alloc());
                    other.PrepareForTree(a);
                    pLeaf->pPrev = 0;
                    for (size_t n = 0; n < nCount - 1; ++n)
                    {
                        pLeaf->arrVals[n] = m_arrVals[n];
                    }
                    m_nState = m_arrVals[nCount - 1];
                    m_pLeaf  = pLeaf->arrVals + nCount - 1;
                    *m_pLeaf = nCount;
                }
                else
                {
                    other.PrepareForTree(a);
                }
                VALUE nCount       = *m_pLeaf + other.m_nState;
                *m_pLeaf++         = m_nState;
                
                pNode->pPrev       = 0;
                pNode->arrLeafs[0] = m_pLeaf;
                pNode->arrLeafs[1] = other.m_pLeaf;
                            
                ffAssume(nCount <= MaxDocumentId + 1);
                
                m_nState = StateStoreCount + nCount;
                m_pNode  = pNode->arrLeafs + 1;
            }                
        }
    }
    
    ffDebugOnly(other.m_nState = StateStoreCount);
    ffAssert(Check());
}

template<typename VALUE, size_t nMAXVAL> 
VALUE Set<VALUE, nMAXVAL>::Count() const throw()
{
    if (StateStoreCount <= m_nState)
    {
        return m_nState - StateStoreCount;
    }

    return *m_pLeaf;
}

template<typename VALUE, size_t nMAXVAL> ffForceInline
bool Set<VALUE, nMAXVAL>::IsExist(VALUE val) const throw()
{
    if (m_nState < StateStoreCount)
    {
        return m_nState == val;
    }
    
    VALUE nCount = m_nState - StateStoreCount;
    if (0 == nCount)
    {
        return false;
    }        
    else if (nCount <= InPlaceItemCount)
    {
        return m_arrVals[nCount - 1] == val;
    }
    else
    {
        const Leaf* pLeaf = GetLeaf(*m_pNode);
        if (pLeaf->arrVals == *m_pNode)
        {
            const Node* pNode = GetNode(m_pNode);
            if (pNode->arrLeafs == m_pNode)
            {//value in last leaf in prev node
                return *(pNode->pPrev->arrLeafs[NodeItemCount - 1] - 1) == val;
            }
            else
            {//value in prev leaf
                return *(*(m_pNode - 1) - 1) == val;
            }
        }
        else
        {//value in current leaf
            return *(*m_pNode - 1) == val;
        }
    }
}

template<typename VALUE, size_t nMAXVAL> 
bool Set<VALUE, nMAXVAL>::HasOneItem() const throw()
{
    ffAssume(m_nState <= StateStoreCount + InPlaceItemCount);
    return StateStoreCount + 1 == m_nState;
}


template<typename VALUE, size_t nMAXVAL> 
template<typename PROC>
void Set<VALUE, nMAXVAL>::Enum(PROC& proc, const VALUE* pValStart) ffThrowAll
{
    for (const VALUE* pVal = pValStart; pVal;)
    {
        const Leaf* pLeaf = GetLeaf(pVal);    
        
        for (const VALUE* p = pVal - 1; p >= pLeaf->arrVals; --p)
        {
            proc(*p);
        }
        
        pVal = pLeaf->pPrev;
    }
}

template<typename VALUE, size_t nMAXVAL> 
template<typename PROC> 
void Set<VALUE, nMAXVAL>::Enum(PROC& proc) const ffThrowAll
{
    if (m_nState < StateStoreCount)
    {//list
        proc(m_nState);
        Enum(proc, m_pLeaf);
    }
    else
    {
        VALUE nCount = m_nState - StateStoreCount;
        if (nCount <= InPlaceItemCount)
        {
            for (;nCount; nCount--)
            {
                proc(m_arrVals[nCount - 1]);
            }
        }
        else
        {//tree
            const Node* pNode = GetNode(m_pNode);
                
            for (VALUE*const* pp = m_pNode; pp >= pNode->arrLeafs; --pp)
            {
                Enum(proc, *pp);
            }        
            
            while (pNode->pPrev)
            {
                pNode = pNode->pPrev;
                for (VALUE*const* pp = pNode->arrLeafs + NodeItemCount - 1; 
                     pp >= pNode->arrLeafs; 
                     --pp)    
                {
                    Enum(proc, *pp);
                }                     
            }            
        }
    }
}

//---------------------------------------------------------------------------

#ifdef ffDebug

template<typename VALUE, size_t nMAXVAL>
size_t Set<VALUE, nMAXVAL>::ValuesCount(VALUE* pLeafRoot) throw()
{
    size_t nCount = 0;

    const Leaf* pLeaf = GetLeaf(pLeafRoot);
    size_t nVals      = VALUE(pLeafRoot - pLeaf->arrVals);
    nCount += nVals;
    
    ffAssume(0 <= nVals && nVals <= LeafItemCount);
    
    while (pLeaf->pPrev)
    {
        const Leaf* pLeafNext = GetLeaf(pLeaf->pPrev);
        nVals = pLeaf->pPrev - pLeafNext->arrVals;
        ffAssume(0 < nVals && nVals <= LeafItemCount);
        nCount += nVals;
        pLeaf = pLeafNext;
    }
    
    return nCount;
}

template<typename VALUE, size_t nMAXVAL>
bool Set<VALUE, nMAXVAL>::Check() const throw()
{
#ifdef ffDebugDeepChecks

    if (m_nState < StateStoreCount)
    {//list
        VALUE nCount = *m_pLeaf;
        ffAssert(ValuesCount(m_pLeaf) + 1 == nCount);        
    }
    else
    {
        VALUE nCount = m_nState - StateStoreCount;
        if (InPlaceItemCount < nCount)
        {//tree
            ffAssume(0 != (size_t(*m_pNode) & BlockIntMask));
        
            size_t nRealCount = 0;
            
            const Node* pNode = GetNode(m_pNode);
            size_t nLeafs = m_pNode - pNode->arrLeafs + 1;
            ffAssume(0 <= nLeafs && nLeafs <= NodeItemCount);
            
            for (size_t n = 0; n < nLeafs; ++n)
            {
                nRealCount += ValuesCount(pNode->arrLeafs[n]);
            }
            
            while (pNode->pPrev)
            {
                pNode = pNode->pPrev;

                for (size_t n = 0; n < NodeItemCount; ++n)
                {
                    nRealCount += ValuesCount(pNode->arrLeafs[n]);
                }
                
            }
                    
            ffAssume(nCount == nRealCount);
        }
        else
        {//internal            
        }
    }
#endif    
    
    return true;
}   

#endif


}//namespace FastFish

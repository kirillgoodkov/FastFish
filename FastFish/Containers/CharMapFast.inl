namespace FastFish{

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> 
CharMapFast<VALUE, bCTRL, bSENS, TRAITS>& CharMapFast<VALUE, bCTRL, bSENS, TRAITS>::operator = (CharMapFast& oth) throw()
{
    m_nCount = oth.m_nCount;
    oth.m_nCount = 0;
    
    if (m_nCount <= ListSize)
    {
        memcpy(m_lst.arrChars, oth.m_lst.arrChars, m_nCount);
        memcpy(m_lst.arrVals, oth.m_lst.arrVals, m_nCount * sizeof(VALUE*));        
    }
    else
    {
        memcpy(&m_map, &oth.m_map, sizeof(m_map));
    }
    
    return *this;
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> ffForceInline
VALUE& CharMapFast<VALUE, bCTRL, bSENS, TRAITS>::AddOrGet(char _ch, AllocatorInvader& a) throw()
{
    using namespace CharMapUtils;
    
    const uns1_t ch = reinterpret_cast<uns1_t&>(_ch);
    ffAssert(IsValidChar(ch));
    
    if (0 == m_nCount)
    {
        m_lst.arrChars[0] = ch;
        m_lst.arrVals[0]  = NewFromRaw<VALUE>(a);
        ++m_nCount;
        return *m_lst.arrVals[0];
    }
    else if (m_nCount <= ListSize)
    {
        size_t nPos = 0;
        for (; nPos < m_nCount; ++nPos)
        {
            if (ch == m_lst.arrChars[nPos])
            {
                return *m_lst.arrVals[nPos];
            }
        }

        VALUE* pResult = NewFromRaw<VALUE>(a);
        
        if (ListSize == m_nCount)
        {
            uns1_t nNewState = uns1_t((Value1R == (ch & Mask1R)) ? stateR : stateAM);
            const uns1_t* arrIndex = TRAITS::s_traits.arrForward[ShiftState(nNewState)];
            size_t nTableSize = TableSize<bCTRL, bSENS>(nNewState);
            VALUE** ppMapNew = static_cast<VALUE**>(a.Alloc(sizeof(VALUE*) * nTableSize));
            memset(ppMapNew, 0, sizeof(VALUE*) * nTableSize);
            for (size_t n = 0; n < ListSize; ++n)
            {
                ffAssert(ch != m_lst.arrChars[n]);
                ppMapNew[arrIndex[m_lst.arrChars[n]]] = m_lst.arrVals[n];
            }
            m_map.ppMap  = ppMapNew;
            m_map.nState = nNewState;
            ffAssert(0 == m_map.ppMap[arrIndex[ch]]);
            m_map.ppMap[arrIndex[ch]] = pResult;    
        }
        else
        {
            ffAssume(nPos < ListSize);
            m_lst.arrVals [nPos] = pResult;
            m_lst.arrChars[nPos] = ch;            
        }
        ++m_nCount;            
        return *pResult;    
    }
    else
    {
        ffAssume(stateAM == m_map.nState || stateR == m_map.nState );
        uns1_t nPos = TRAITS::s_traits.arrForward[ShiftState(m_map.nState)][ch];
        ffAssert(0 < nPos && nPos < (TableSize<bCTRL, bSENS>(m_map.nState)));

        VALUE*& pVal = m_map.ppMap[nPos];
        if (0 == pVal)    
        {
            pVal = NewFromRaw<VALUE>(a);
            ++m_nCount;
        }
        
        return *pVal;                
    }
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> ffForceInline
VALUE* CharMapFast<VALUE, bCTRL, bSENS, TRAITS>::Find(char _ch) const throw()
{
    using namespace CharMapUtils;
    
    const uns1_t ch = reinterpret_cast<uns1_t&>(_ch);
    ffAssert(IsValidChar(ch));
    
    if (0 == m_nCount)
    {
        return 0;
    }
    else if (m_nCount <= ListSize)
    {
        for (size_t nPos = 0; nPos < m_nCount; ++nPos)
        {
            if (ch == m_lst.arrChars[nPos])
            {
                return m_lst.arrVals[nPos];
            }
        }
        return 0;
    }
    else
    {
        ffAssume(stateAM == m_map.nState || stateR == m_map.nState );
        uns1_t nPos = TRAITS::s_traits.arrForward[ShiftState(m_map.nState)][ch];
        ffAssert(0 < nPos && nPos < (TableSize<bCTRL, bSENS>(m_map.nState)));
        return m_map.ppMap[nPos];                
    }
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS>
template<typename PROC>
void CharMapFast<VALUE, bCTRL, bSENS, TRAITS>::Enum(PROC& proc) const ffThrowAll
{
    using namespace CharMapUtils;
    
    if (m_nCount <= ListSize)
    {//list
        for (size_t n = 0; n < m_nCount; ++n)
        {
            ffAssert(IsValidChar(m_lst.arrChars[n]));
            proc(m_lst.arrChars[n], m_lst.arrVals[n]);
        }
    }
    else 
    {//direct mapping
        ffAssume(stateAM == m_map.nState || stateR == m_map.nState );
        size_t nTableSize = TableSize<bCTRL, bSENS>(m_map.nState);
        const uns1_t* arrIndex = TRAITS::s_traits.arrBackward[ShiftState(m_map.nState)];
        for (size_t n = 1; n < nTableSize; ++n)
        {
            if (m_map.ppMap[n])
            {
                proc(arrIndex[n], m_map.ppMap[n]);
            }                
        }
    }    
}


}//namespace FastFish

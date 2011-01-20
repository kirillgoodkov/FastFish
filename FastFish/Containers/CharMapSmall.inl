#include "Tools/Bits.h"
#include "Tools/Tools.h"

namespace FastFish{

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> 
CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::CharMapSmall() throw():
    m_nCount(0),
    m_nState(0),
    m_pVal(0)
#ifdef ffDebug
    ,m_nDebugCount(0)
#endif
{
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> 
void CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::FillMask(uns1_t arrMask[]) const throw()
{
    using namespace CharMapUtils;
    
    if (0 < m_nState && m_nState < stateList)
    {
        uns1_t ch = m_nState;
        SetBit(arrMask, ch);
    }
    else if (stateList == m_nState)
    {
        for (size_t n = 0; n < m_nCount; ++n)
        {
            ffAssert(0 != m_pList->arrValuePtr[n]);            
            uns1_t ch = m_pList->arrChars[n];
            SetBit(arrMask, ch);
        }
    }
    else
    {
        ffAssume(stateA <= m_nState);
        size_t nTableSize = TableSize<bCTRL, bSENS>(m_nState);
        const uns1_t* arrIndex = TRAITS::s_traits.arrBackward[ShiftState(m_nState)];
        for (size_t n = 1; n < nTableSize; ++n)
        {
            if (m_ppMap[n])
            {
                uns1_t ch = arrIndex[n];
                ffAssume(0 != ch);
                SetBit(arrMask, ch);
            }
        }
    }
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> 
void CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::InitNewMapFromOld(VALUE** ppMapNew) const throw()
{
    using namespace CharMapUtils;
    ffAssume(stateA == m_nState || stateM == m_nState);
    ppMapNew[0] = 0;
    ffAssert(0 == m_ppMap[0]);
    
    VALUE**      ppDst = ppMapNew + 1;
    VALUE*const* ppSrc = m_ppMap  + 1;
    
    if (stateA == m_nState)
    {
        memcpy(ppDst, ppSrc, sizeof(VALUE*) * CountA);
        ppSrc += CountA;
    }
    else
    {
        memset(ppDst, 0, sizeof(VALUE*) * CountA);
    }
    ppDst += CountA;

    if (stateM == m_nState)
    {
        memcpy(ppDst, ppSrc, sizeof(VALUE*) * SizeM);
        ffDebugOnly(ppSrc += SizeM);
    }
    else
    {
        memset(ppDst, 0, sizeof(VALUE*) * SizeM);
    }    
    ffDebugOnly(ppDst += SizeM);
    
    ffAssert(m_nDebugCount == size_t(ppDst - ppMapNew));    
}

//----------------------------------------------------------------------------

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> ffForceInline
VALUE& CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::AddOrGet(char _ch, CharMapAllocator& a) throw()
{
    using namespace CharMapUtils;

    const uns1_t ch = reinterpret_cast<uns1_t&>(_ch);
    ffAssert(IsValidChar(ch));
    
    if (0 == m_nState)
    {
        m_pVal   = New<VALUE>(a.aItem);
        m_nState = ch;
        ffDebugOnly(m_nDebugCount = 1);
        ++m_nCount;
        return *m_pVal;
    }
    else if (m_nState < stateList)//single item
    {
        if (ch == m_nState)
        {
            ffAssume(m_pVal);
            return *m_pVal;
        }
        else
        {
            VALUE* pValNew = New<VALUE>(a.aItem);
            VALUE* pValOld = m_pVal;
            m_pList        = static_cast<List*>(a.aList.Alloc());
            ffDebugOnly(m_nDebugCount = ListSize);

            m_pList->arrChars   [0] = m_nState;
            m_pList->arrValuePtr[0] = pValOld;
            m_pList->arrChars   [1] = ch;
            m_pList->arrValuePtr[1] = pValNew;
            
            ++m_nCount;
            ffAssume(2 == m_nCount);
            m_nState = stateList;
            return *m_pList->arrValuePtr[1];
        }
    }
    else if (stateList == m_nState)//list
    {
        size_t nPos = 0;
        for (; nPos < m_nCount; ++nPos)
        {
            ffAssert(nPos < m_nDebugCount);
            if (ch == m_pList->arrChars[nPos])
            {
                return *m_pList->arrValuePtr[nPos];
            }
        }

        VALUE* pResult = New<VALUE>(a.aItem);
        
        if (ListSize == m_nCount)
        {
            ffAssumeStatic(ListSize < sizeof(m_pList->nChars8));
            m_pList->arrChars[ListSize] = ch;
            uns8_t nUpper = m_pList->nChars8 & Mask8Upper;
            uns8_t nUpperShift = nUpper >> 1;

            uns1_t nNewState = ffCreateState(Mask8Upper != nUpper,              //A
                                             m_pList->nChars8 & nUpperShift,    //M
                                             ~m_pList->nChars8 & nUpperShift);  //R
                                             
            const uns1_t* arrIndex = TRAITS::s_traits.arrForward[ShiftState(nNewState)];
            
            VALUE** ppMapNew = a.TableAllocNulls<VALUE, bCTRL, bSENS>(nNewState);
            ffDebugOnly((m_nDebugCount = TableSize<bCTRL, bSENS>(nNewState)));
            ffAssert(ListSize < m_nDebugCount);
            for (size_t n = 0; n < ListSize; ++n)
            {
                ffAssert(0 != arrIndex[m_pList->arrChars[n]]);
                ffAssert(arrIndex[m_pList->arrChars[n]] < m_nDebugCount);
                ppMapNew[arrIndex[m_pList->arrChars[n]]] = m_pList->arrValuePtr[n];
            }
            a.aList.Free(m_pList);
            m_ppMap  = ppMapNew;
            m_nState = nNewState;            
            ffAssert(0 < arrIndex[ch] && arrIndex[ch] < m_nDebugCount);
            ffAssume(0 == m_ppMap[arrIndex[ch]]);
            m_ppMap[arrIndex[ch]] = pResult;
            ffAssert(0 == m_ppMap[0]);
        }
        else //list not full, nPos - first empty position
        {
            ffAssert(nPos < m_nDebugCount);
            ffAssume(nPos < ListSize);
            m_pList->arrValuePtr[nPos] = pResult;
            m_pList->arrChars   [nPos] = ch;
        }
        ++m_nCount;            
        return *pResult;
    }
    else //direct mapping
    {
        ffAssert(0 == m_ppMap[0]);
        ffAssume(stateA <= m_nState);
        uns1_t nPos = TRAITS::s_traits.arrForward[ShiftState(m_nState)][ch];
        if (0 == nPos)
        {
            ffAssume(!(stateR == m_nState || stateAM == m_nState));
            ffAssert(Value1R != (ch & Mask1R));
            
            VALUE** ppMapNew = a.TableAlloc<VALUE, bCTRL, bSENS>(stateAM);
            ffDebugOnly((m_nDebugCount = TableSize<bCTRL, bSENS>(stateAM)));
            
            InitNewMapFromOld(ppMapNew);            
            
            a.TableFree<VALUE, bCTRL, bSENS>(m_nState, m_ppMap);
            
            m_ppMap  = ppMapNew;                    
            m_nState = stateAM;
            nPos     = TRAITS::s_traits.arrForward[ShiftState(stateAM)][ch];
            ffAssume(0 != nPos);                           
        }
        
        ffAssert(0 < nPos && nPos < m_nDebugCount);
        VALUE*& pVal = m_ppMap[nPos];
        if (0 == pVal)    
        {
            pVal = New<VALUE>(a.aItem);
            ++m_nCount;
        }
        else
        {
            a.aItem.Check(pVal);
        }
        ffAssert(0 == m_ppMap[0]);
        return *pVal;        
    }    
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> 
CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>& CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::operator = (CharMapSmall& oth) throw()
{
    m_nCount = oth.m_nCount;
    m_nState = oth.m_nState;
    m_pVal   = oth.m_pVal;
    ffDebugOnly(m_nDebugCount = oth.m_nDebugCount);

    oth.m_nCount = 0;
    oth.m_nState = 0;
    oth.m_pVal   = 0;
    ffDebugOnly(oth.m_nDebugCount = 0);
    
    return *this;
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS> 
const VALUE* CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::GetFirst() const throw()
{
    ffAssume(0 != m_nState);
    
    using namespace CharMapUtils;
    if (m_nState < stateList)
    {
        return m_pVal;
    }
    else if (m_nState < stateA)
    {
        size_t nMin = 0;        
        
        for (size_t n = 1; n < m_nCount; ++n)
        {
            if (m_pList->arrChars[n] < m_pList->arrChars[nMin])
            {
                nMin = n;
            }                
        }
        return m_pList->arrValuePtr[nMin];        
    }
    else //direct mapping
    {
        size_t nTableSize = TableSize<bCTRL, bSENS>(m_nState);
        for (size_t n = 1; n < nTableSize; ++n)
        {
            if (m_ppMap[n])
            {
                return m_ppMap[n];
            }                
        }
        ffAssume(false);
    }    
    ffReturnNop;
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS>
template<typename PROC>
void CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::Enum(PROC& proc) ffThrowAll
{
    if (0 == m_nState)
    {
        return;
    }

    using namespace CharMapUtils;
    if (m_nState < stateList)
    {
        proc(m_nState, m_pVal);
    }
    else if (m_nState < stateA)
    {
        size_t arrIndex[] = {0, 1, 2, 3, 4, 5, 6};
        ffAssumeStatic(ListSize <= ffCountOf(arrIndex));
        InderectCmp<uns1_t> cmp(m_pList->arrChars);
        std::sort(arrIndex, arrIndex + m_nCount, cmp);
    
        for (size_t n = 0; n < m_nCount; ++n)
        {
            proc(m_pList->arrChars[arrIndex[n]], m_pList->arrValuePtr[arrIndex[n]]);
        }
    }
    else //direct mapping
    {
        size_t nTableSize = TableSize<bCTRL, bSENS>(m_nState);
        const uns1_t* arrIndex = TRAITS::s_traits.arrBackward[ShiftState(m_nState)];
        for (size_t n = 1; n < nTableSize; ++n)
        {
            if (m_ppMap[n])
            {
                proc(arrIndex[n], m_ppMap[n]);
            }                
        }
    }    
}

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS>
template<typename ARGTYPE> ffForceInline
void CharMapSmall<VALUE, bCTRL, bSENS, TRAITS>::Merge(CharMapSmall& other, CharMapAllocator& a, ARGTYPE& arg) throw()
{
    if (other.IsEmpty())
    {
        return;
    }
    else if (IsEmpty())
    {
        *this = other;
        return;
    }
    
    using namespace CharMapUtils;
    
    size_t nNewCount = 0;
    
    {//enlarge map
    
        uns8_t arrMaskT[256/64] = {0};
        FillMask(reinterpret_cast<uns1_t*>(arrMaskT));

        uns8_t arrMaskO[256/64] = {0};
        other.FillMask(reinterpret_cast<uns1_t*>(arrMaskO));

        nNewCount = CountBits(arrMaskT[0] | arrMaskO[0]) + 
                    CountBits(arrMaskT[1] | arrMaskO[1]) + 
                    CountBits(arrMaskT[2] | arrMaskO[2]) + 
                    CountBits(arrMaskT[3] | arrMaskO[3]) ;

        ffAssert(CountBits(arrMaskT[0]) + CountBits(arrMaskT[1]) + CountBits(arrMaskT[2]) + CountBits(arrMaskT[3]) == m_nCount);

        /*  reallocations:
        1->list
        1->map
        list->map
        map?->map (if old state non terminal & new state != old)
        */

        
        if (0 < m_nState && m_nState < stateList && 1 < nNewCount && nNewCount <= ListSize)
        {//1->list    
            VALUE* pValOld = m_pVal;
            m_pList        = static_cast<List*>(a.aList.Alloc());
            ffDebugOnly(m_nDebugCount = ListSize);

            m_pList->arrChars   [0] = m_nState;
            m_pList->arrValuePtr[0] = pValOld;    
            m_nState                = stateList;
        }
        else if (ListSize < nNewCount)
        {//(1,list, map?)->map        
            if (m_nState < stateA)
            {//(1, list) -> map
                uns1_t nNewState = ffCreateState((arrMaskT[0]|arrMaskO[0]) || (arrMaskT[1]|arrMaskO[1]), //A
                                                 arrMaskT[3]|arrMaskO[3],                                //M
                                                 arrMaskT[2]);                                           //R
                VALUE** ppMapNew = a.TableAllocNulls<VALUE, bCTRL, bSENS>(nNewState);
                ffDebugOnly((m_nDebugCount = TableSize<bCTRL, bSENS>(nNewState)));
                if (m_nState < stateList)
                {//1->map
                    ppMapNew[TRAITS::s_traits.arrForward[ShiftState(nNewState)][m_nState]] = m_pVal;
                }
                else
                {//list->map
                    const uns1_t* arrIndex = TRAITS::s_traits.arrForward[ShiftState(nNewState)];
                    for (size_t n = 0; n < m_nCount; ++n)
                    {
                        ppMapNew[arrIndex[m_pList->arrChars[n]]] = m_pList->arrValuePtr[n];
                    }
                    a.aList.Free(m_pList);                
                }
                m_ppMap  = ppMapNew;
                m_nState = nNewState;
            }
            else if (stateA == m_nState || stateM == m_nState)
            {//map?->map
                uns1_t nNewState = ffCreateState((arrMaskT[0]|arrMaskO[0]) || (arrMaskT[1]|arrMaskO[1]), //A
                                                 arrMaskT[3]|arrMaskO[3],                                //M
                                                 false);                                                 //R
                if (nNewState != m_nState)
                {
                    VALUE** ppMapNew = a.TableAlloc<VALUE, bCTRL, bSENS>(nNewState);
                    ffDebugOnly((m_nDebugCount = TableSize<bCTRL, bSENS>(nNewState)));

                    InitNewMapFromOld(ppMapNew);

                    a.TableFree<VALUE, bCTRL, bSENS>(m_nState, m_ppMap);
                    m_nState = nNewState;
                    m_ppMap  = ppMapNew;                
                }
            }

        }
    }
    //-----------------------------------------------------------------------
    
    if (1 == nNewCount)
    {//1, 1
        ffAssume(1 == other.m_nCount);
        ffAssume(m_nState == other.m_nState);
        m_pVal->Merge(*other.m_pVal, arg);
    }
    else if (nNewCount <= ListSize)
    {//list, *
        if (other.m_nState < stateList)
        {//list, 1
            ffAssume(0 != m_nState);
            
            size_t n = 0;
            for (; n < m_nCount; ++n)
            {
                if (m_pList->arrChars[n] == other.m_nState)
                {
                    m_pList->arrValuePtr[n]->Merge(*other.m_pVal, arg);
                    break;
                }
            }
            ffAssume(n < ListSize);
            if (n == m_nCount)
            {
                m_pList->arrChars[n]    = other.m_nState;
                m_pList->arrValuePtr[n] = other.m_pVal;
                ++m_nCount;
            }
        }
        else
        {//list, list
            ffAssume(stateList == other.m_nState);
            //merge lists
            size_t nOldTCount = m_nCount;
            for (size_t nO = 0; nO < other.m_nCount; ++nO)
            {
                uns1_t ch   = other.m_pList->arrChars[nO];
                VALUE* pVal = other.m_pList->arrValuePtr[nO];
                size_t nT = 0;
                for (; nT < nOldTCount; ++nT)
                {
                    if (m_pList->arrChars[nT] == ch)
                    {
                        m_pList->arrValuePtr[nT]->Merge(*pVal, arg);
                        break;
                    }
                }
                if (nT == nOldTCount)
                {
                    m_pList->arrChars[m_nCount]    = ch;
                    m_pList->arrValuePtr[m_nCount] = pVal;
                    ++m_nCount;
                }
            }                            
            ffAssume(m_nCount <= ListSize);
            a.aList.Free(other.m_pList);            
        }
    }
    else
    {
        ffAssume(stateA <= m_nState);
        if (other.m_nState < stateList)
        {//map,1
            ffAssume(0 != other.m_nState);
            
            size_t nPos = TRAITS::s_traits.arrForward[ShiftState(m_nState)][other.m_nState];
            ffAssert(0 < nPos && nPos < m_nDebugCount);
            VALUE*& pVal = m_ppMap[nPos];
            if (0 == pVal)
            {
                pVal = other.m_pVal;
                ++m_nCount;
            }
            else
            {
                pVal->Merge(*other.m_pVal, arg);
            }
        }
        else if (stateList == other.m_nState)
        {//map, list
            const uns1_t* arrIndex = TRAITS::s_traits.arrForward[ShiftState(m_nState)];
            for (size_t n = 0; n < other.m_nCount; ++n)
            {
                size_t nPos = arrIndex[other.m_pList->arrChars[n]];
                ffAssert(0 < nPos && nPos < m_nDebugCount);
                VALUE*& pVal = m_ppMap[nPos];    
                if (0 == pVal)
                {
                    pVal = other.m_pList->arrValuePtr[n];
                    ++m_nCount;
                }
                else
                {
                    pVal->Merge(*other.m_pList->arrValuePtr[n], arg);
                }
            }
            a.aList.Free(other.m_pList);
        }
        else
        {//map, map
            ffAssume(stateA <= other.m_nState);
            const uns1_t* arrIndexBack = TRAITS::s_traits.arrBackward[ShiftState(other.m_nState)];
            const uns1_t* arrIndexForw = TRAITS::s_traits.arrForward[ShiftState(m_nState)];
            
            size_t nCount = TableSize<bCTRL, bSENS>(other.m_nState);
            for (size_t n = 0; n < nCount; ++n)
            {
                VALUE* pOther = other.m_ppMap[n];
                if (pOther)
                {
                    ffAssert(0 != arrIndexBack[n]);
                    size_t nPos = arrIndexForw[arrIndexBack[n]];
                    ffAssert(0 < nPos && nPos < m_nDebugCount);
                    VALUE*& pVal = m_ppMap[nPos];    
                    if (0 == pVal)
                    {
                        pVal = pOther;
                        ++m_nCount;
                    }
                    else
                    {
                        pVal->Merge(*pOther, arg);
                    }                    
                }                   
            }
            a.TableFree<VALUE, bCTRL, bSENS>(other.m_nState, other.m_ppMap);            
        }
    }   
    ffAssume(m_nCount == nNewCount);
}

}//namespace FastFish

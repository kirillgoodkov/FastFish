
//define is used for lazy evaluation _EXP*
#define ffCreateState(_EXPA, _EXPM, _EXPR)\
    uns1_t( (_EXPR) ?\
                    stateR :\
                    ( (_EXPM) ?\
                              ( (_EXPA) ?\
                                        stateAM :\
                                        stateM):\
                              stateA))

namespace FastFish{

namespace CharMapUtils
{
    inline
    uns1_t ShiftState(uns1_t nState) throw()
    {
        ffAssume(stateA <= nState);   
        return uns1_t(nState - stateA);
    }

    inline
    bool IsValidChar(uns1_t ch) throw()
    {
        return !(0 == ch || InvalidChar0 == ch || InvalidChar1 == ch || RestrictedCharsBegin <= ch);
    }
    
    template<bool bCTRL, bool bSENS>
    size_t TableSize(uns1_t nState) throw()
    {
        using namespace CharMapUtils;   
        ffAssume(stateA <= nState);
        
        switch (nState)
        {
            case stateA:
                return 1 + SizeA + (bCTRL ? SizeC : 0) + (bSENS ? SizeS : 0);
            case stateM:
                return 1 + SizeM;
            case stateR:
                return 1 + SizeR;
            default:
                ffAssume(nState == stateAM);
                return 1 + SizeA + (bCTRL ? SizeC : 0) + (bSENS ? SizeS : 0) + SizeM;
        }
    }
    
    
}

template<typename VALUE, bool bCTRL, bool bSENS>
VALUE** CharMapAllocator::TableAlloc(uns1_t nState) throw()
{
    using namespace CharMapUtils;
    ffAssume(stateA <= nState);
    
    void* p = 0;
    switch (nState)
    {
        case stateR:
            p = m_aR.Alloc();
            break;
        case stateM:
            p = m_aM.Alloc(); 
            break;
        default:
        {
            ffAssume(stateA == nState || stateAM == nState);
            OptionDepend& od = bCTRL ? (bSENS ? m_od11 : m_od10) :
                                       (bSENS ? m_od01 : m_od00) ;
            p = (stateA == nState) ? od.aA.Alloc() : od.aAM.Alloc();
        }
    }
    return static_cast<VALUE**>(p);
}

template<typename VALUE, bool bCTRL, bool bSENS>
VALUE** CharMapAllocator::TableAllocNulls(uns1_t nState) throw()
{
    using namespace CharMapUtils;
    VALUE** p = TableAlloc<VALUE, bCTRL, bSENS>(nState);
    memset(p, 0, sizeof(VALUE*) * TableSize<bCTRL, bSENS>(nState));
    return p;
}

template<typename VALUE, bool bCTRL, bool bSENS>
void CharMapAllocator::TableFree(uns1_t nState, VALUE** p) throw()
{
    using namespace CharMapUtils;
    ffAssume(stateA <= nState);

    switch (nState)
    {
        case stateR:
            m_aR.Free(p);
            break;
        case stateM:
            m_aM.Free(p); 
            break;
        default:
        {
            ffAssume(stateA == nState || stateAM == nState);
            OptionDepend& od = bCTRL ? (bSENS ? m_od11 : m_od10) :
                                       (bSENS ? m_od01 : m_od00) ;
            if (stateA == nState)
                od.aA.Free(p);
            else
                od.aAM.Free(p);
        }
    }            
}

}//namespace FastFish

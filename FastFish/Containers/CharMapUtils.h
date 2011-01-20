#pragma once
#include "Memory/AllocatorSwapper.h"
#include "Memory/AllocatorInvader.h"
#include "Common/Common.h"

namespace FastFish{

/*
Abbreviations:
    A - ASCII symbols [0 - 80)
    M - begin of multi-byte sequence [C2-F4]
    R - rest of multi-byte sequence [80-BF]
    
    C - controls char block [01-20)
    S - case sensitive, block of chars from 'A' till 'Z' [41 - 5A]
*/

namespace CharMapUtils
{
    const size_t SizeC  = 31;
    const size_t SizeS  = 26;
    const size_t SizeA  = 127 - SizeC - SizeS;

    const size_t SizeM  = 51;
    const size_t SizeR  = 64;      
    
    const uns1_t RestrictedCharsBegin = 0xF5;
    const uns1_t InvalidChar0         = 0xC0;
    const uns1_t InvalidChar1         = 0xC1;

    const uns1_t Mask1A     = 0x80;    
    const uns1_t Value1A    = 0x00;    
    const uns1_t Mask1M     = 0xC0;    
    const uns1_t Value1M    = 0xC0;    
    const uns1_t Mask1R     = 0xC0;    
    const uns1_t Value1R    = 0x80;    

    enum State
    {
        stateList = RestrictedCharsBegin,
        stateA,
        stateM,
        stateR,    //terminal state
        stateAM,   //terminal state
        statesCount = 4
    };        
    
    uns1_t ShiftState(uns1_t nState) throw();
    
    template<bool bCTRL, bool bSENS>
    size_t TableSize(uns1_t nState) throw();    
    
    bool IsValidChar(uns1_t ch) throw();    
}

class CharMapAllocator
{
public:
    CharMapAllocator(size_t nValueSize) throw();
    
    AllocatorSwapper    aItem;
    AllocatorSwapper    aList;

    template<typename VALUE, bool bCTRL, bool bSENS>
    VALUE** TableAlloc(uns1_t nState) throw();

    template<typename VALUE, bool bCTRL, bool bSENS>
    VALUE** TableAllocNulls(uns1_t nState) throw();
    
    template<typename VALUE, bool bCTRL, bool bSENS>
    void    TableFree(uns1_t nState, VALUE** p) throw();
    
private:    

    AllocatorSwapper    m_aR;
    AllocatorSwapper    m_aM;
    
    struct OptionDepend
    {
        AllocatorSwapper  aA;
        AllocatorSwapper  aAM;

        OptionDepend(bool bCtrl, bool bSens) throw();
    };
    
    OptionDepend        m_od00;
    OptionDepend        m_od01;
    OptionDepend        m_od10;
    OptionDepend        m_od11;    
};

template<bool bCTRL, bool bSENS>
struct CharMapTraits
{
    static const CharMapTraits s_traits;

    uns1_t arrForward[CharMapUtils::statesCount][SizeUns1];        
    uns1_t arrBackward[CharMapUtils::statesCount][SizeUns1];        

    CharMapTraits() throw();    
};

}//namespace FastFish
#include "Containers/CharMapUtils.inl"

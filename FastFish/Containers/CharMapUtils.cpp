#include "Containers/CharMapSmall.h"
#include "Common/Consts.h"

namespace FastFish{

using namespace CharMapUtils;

typedef CharMapSmall<void*, false, false> CharMapGen;

const size_t SizeOD[2][2] = 
{
    {1 + SizeA,         1 + SizeA         + SizeS},
    {1 + SizeA + SizeC, 1 + SizeA + SizeC + SizeS}    
};

CharMapAllocator::CharMapAllocator(size_t nValueSize) throw():
    aItem (PC::SmallCharMapAllocItemMul * nValueSize                , nValueSize                 , nValueSize), 
    aList (PC::SmallCharMapAllocListMul * sizeof(CharMapGen::List)  , sizeof(CharMapGen::List)   , sizeof(CharMapGen::List)),
    
    m_aR  (PC::SmallCharMapAllocMapMul * (1 + SizeR) * sizeof(void*), (1 + SizeR) * sizeof(void*), sizeof(void*)),
    m_aM  (PC::SmallCharMapAllocMapMul * (1 + SizeM) * sizeof(void*), (1 + SizeM) * sizeof(void*), sizeof(void*)), 

    m_od00(false, false),
    m_od01(false, true),
    m_od10(true, false),
    m_od11(true, true)
{
    ffAssumeStatic(0 == (CacheLine % sizeof(CharMapGen::List)));
}

CharMapAllocator::OptionDepend::OptionDepend(bool bCtrl, bool bSens) throw():
    aA (PC::SmallCharMapAllocMapMul *  SizeOD[bCtrl][bSens]          * sizeof(void*),  SizeOD[bCtrl][bSens]          * sizeof(void*), sizeof(void*)),
    aAM(PC::SmallCharMapAllocMapMul * (SizeOD[bCtrl][bSens] + SizeM) * sizeof(void*), (SizeOD[bCtrl][bSens] + SizeM) * sizeof(void*), sizeof(void*))
{
}

template<bool bCTRL, bool bSENS>
CharMapTraits<bCTRL, bSENS>::CharMapTraits() throw()
{
    using namespace CharMapUtils;
    
    memset(arrForward, 0 , sizeof(arrForward));
    memset(arrBackward, 0 , sizeof(arrBackward));

    uns1_t arrIdx[SizeUns1];
    memset(arrIdx, 1, sizeof(arrIdx));         
    
    if (Static(bCTRL))
    {
        for (uns1_t n = 0x01; n < 0x20; ++n)
        {//A*
            arrBackward[ShiftState(stateA )][arrIdx[stateA ]] = n;
            arrBackward[ShiftState(stateAM)][arrIdx[stateAM]] = n;
        
            arrForward[ShiftState(stateA )][n] = arrIdx[stateA ]++;
            arrForward[ShiftState(stateAM)][n] = arrIdx[stateAM]++;
        }            
    }        
        
    for (uns1_t n = 0x20; n < 'A'; ++n)          
    {//A*
        arrBackward[ShiftState(stateA )][arrIdx[stateA ]] = n;
        arrBackward[ShiftState(stateAM)][arrIdx[stateAM]] = n;

        arrForward[ShiftState(stateA )][n] = arrIdx[stateA ]++;
        arrForward[ShiftState(stateAM)][n] = arrIdx[stateAM]++;
    }
    
    if (Static(bSENS))
    {
        for (uns1_t n = 'A'; n <= 'Z'; ++n)
        {//A*
            arrBackward[ShiftState(stateA )][arrIdx[stateA ]] = n;
            arrBackward[ShiftState(stateAM)][arrIdx[stateAM]] = n;
            
            arrForward[ShiftState(stateA )][n] = arrIdx[stateA ]++;
            arrForward[ShiftState(stateAM)][n] = arrIdx[stateAM]++;
        }
    }
    
    for (uns1_t n = 'Z' + 1; n < 0x80; ++n)
    {//A*
        arrBackward[ShiftState(stateA )][arrIdx[stateA ]] = n;
        arrBackward[ShiftState(stateAM)][arrIdx[stateAM]] = n;
        
        arrForward[ShiftState(stateA )][n] = arrIdx[stateA ]++;
        arrForward[ShiftState(stateAM)][n] = arrIdx[stateAM]++;
    }

    for (uns1_t n = 0x80; n <= 0xBF; ++n)
    {//R*
        arrBackward[ShiftState(stateR)][arrIdx[stateR]] = n;
        
        arrForward[ShiftState(stateR)][n] = arrIdx[stateR]++;
    }

    for (uns1_t n = 0xC2; n <= 0xF4; ++n)
    {//M*
        arrBackward[ShiftState(stateM )][arrIdx[stateM ]] = n;
        arrBackward[ShiftState(stateAM)][arrIdx[stateAM]] = n;
        
        arrForward[ShiftState(stateM )][n] = arrIdx[stateM ]++;
        arrForward[ShiftState(stateAM)][n] = arrIdx[stateAM]++;
    }
}

template<bool bCTRL, bool bSENS>
const CharMapTraits<bCTRL, bSENS> CharMapTraits<bCTRL, bSENS>::s_traits;

template struct CharMapTraits<false, false>;
template struct CharMapTraits<false, true >;
template struct CharMapTraits<true , false>;
template struct CharMapTraits<true , true >;

}//namespace FastFish

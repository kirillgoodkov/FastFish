#pragma once
#include "Containers/CharMapUtils.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{

#pragma pack(push, 1)
template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS = CharMapTraits<bCTRL, bSENS> >
class CharMapSmall
{
    friend class CharMapAllocator;
    CharMapSmall(const CharMapSmall&);
public:
    CharMapSmall() throw();

    CharMapSmall& operator = (CharMapSmall&) throw();

    VALUE& AddOrGet(char ch, CharMapAllocator& a) throw();

    template<typename ARGTYPE>
    void Merge(CharMapSmall& other, CharMapAllocator& a, ARGTYPE& arg1) throw();
    
    uns1_t  Count()         const throw()           {return m_nCount;}
    bool    IsEmpty()       const throw()           {return 0 == m_nState;}
    const VALUE* GetFirst() const throw();


    template<typename PROC>
    void Enum(PROC& proc)   const ffThrowAll        {const_cast<CharMapSmall*>(this)->Enum(proc);}

    template<typename PROC>
    void Enum(PROC& proc)         ffThrowAll;

private:
    void FillMask(uns1_t arrMask[])          const throw();
    void InitNewMapFromOld(VALUE** ppMapNew) const throw();

    static const uns8_t Mask8Upper =
                            #ifdef ffLittleEndian
                                    ff32Only(0x0080808080808080ull)
                            #else // BigEndian
                                    ff32Only(0x8080808080808000ull)
                            #endif                             
                                    ff64Only(0x8080808080808080ull);    
    static const size_t ListSize   = PC::SmallCharMapListSize;    
    static const uns1_t CountA     = CharMapUtils::SizeA + (bCTRL ? CharMapUtils::SizeC : 0) + (bSENS ? CharMapUtils::SizeS : 0);
    
    struct List
    {
        union
        {
            uns8_t  nChars8;  
            uns1_t  arrChars[ListSize + 1];
        };
        VALUE*  arrValuePtr[ListSize];
    };
    
    uns1_t  m_nCount;
    uns1_t  m_nState;

    union
    {
        VALUE*  m_pVal;    
        List*   m_pList;
        VALUE** m_ppMap;
    };

    ffDebugOnly(size_t m_nDebugCount);    
};
#pragma pack(pop)

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS = CharMapTraits<bCTRL, bSENS> >
struct CharMapSmallPd
{
    void*                                     pad0;      
    uns2_t                                    pad1;
    CharMapSmall<VALUE, bCTRL, bSENS, TRAITS> val;
};

}//namespace FastFish

#include "Containers/CharMapSmall.inl"


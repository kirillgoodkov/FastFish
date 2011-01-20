#pragma once
#include "Containers/CharMapUtils.h"
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish{

#pragma pack(push, 1)
template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS = CharMapTraits<bCTRL, bSENS> >
class CharMapFast
{
    CharMapFast(const CharMapFast&);
public:
    CharMapFast() throw()                                       : m_nCount(0) {}

    CharMapFast& operator = (CharMapFast&) throw();         

    VALUE& AddOrGet(char ch, AllocatorInvader& a) throw();
    VALUE* Find(char ch) const throw();

    uns1_t Count() const throw()                                {return m_nCount;}
    bool   IsEmpty() const throw()                              {return 0 == m_nCount;}

    template<typename PROC>
    void Enum(PROC& proc) const ffThrowAll;

private:
    static const size_t ListSize = PC::FastCharMapListSize;
    
    struct List
    {
        uns1_t  arrChars[ListSize];
        VALUE*  arrVals [ListSize];
    };
    
    struct Map
    {
        uns1_t  nState;
        
        ff64Only(byte_t  pad[4]);
        VALUE** ppMap;
    };
    
    uns1_t      m_nCount;
    
    union
    {
        List m_lst;
        Map  m_map;
    };
    
};
#pragma pack(pop)

template<typename VALUE, bool bCTRL, bool bSENS, typename TRAITS = CharMapTraits<bCTRL, bSENS> >
struct CharMapFastPd
{
    void*                                    pad0;      
    uns2_t                                   pad1;
    CharMapFast<VALUE, bCTRL, bSENS, TRAITS> val;
    
    CharMapFastPd() {};
};


}//namespace FastFish

#include "Containers/CharMapFast.inl"

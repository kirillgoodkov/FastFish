#pragma once
#include "Common/Templates.h"
#include "Common/Common.h"
    
namespace FastFish {

struct TplFieldSearchTextContext
{
    std::vector<std::string> m_vecStr;
    std::vector<const char*> m_vecPtr;
};         

OFStream& operator << (OFStream& file, const TplFieldDataFix& tpl)     ffThrowAll;
IFStream& operator >> (IFStream& file, TplFieldDataFix& tpl)           throw();

OFStream& operator << (OFStream& file, const TplFieldDataVar& tpl)     ffThrowAll;
IFStream& operator >> (IFStream& file, TplFieldDataVar& tpl)           throw();

void LoadTpl(IFStream& file, TplFieldSearchText& tpl, TplFieldSearchTextContext& ctx) throw();
OFStream& operator << (OFStream& file, const TplFieldSearchText& tpl)                 ffThrowAll;

OFStream& operator << (OFStream& file, const TplIndex& tpl)           ffThrowAll;

class TplIndexLoader:
    public TplIndex
{
public:
    TplIndexLoader() throw() {memset(arrFieldAS, 0, sizeof(arrFieldAS));};
    void Load(IFStream& file) throw();
    
    uns1_t arrFieldAS[(MaxFieldId + BitsInByte - 1)/sizeof(BitsInByte)];
private:

    std::vector<TplFieldDataFix>            m_vecDataFix;
    std::vector<TplFieldDataVar>            m_vecDataVar;
    std::vector<TplFieldSearchText>         m_vecSearchText;
    std::vector<TplFieldSearchTextContext>  m_vecSearchTextContext;
};

}//namespace FastFish 


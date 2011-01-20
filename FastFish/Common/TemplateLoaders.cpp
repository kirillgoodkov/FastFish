#include "Common/TemplateLoaders.h"
#include "Tools/OFStream.h"
#include "Tools/IFStream.h"
#include "Tools/Bits.h"
#include "Tools/Tools.h"

using namespace std;

namespace FastFish{

//----------------------------------------------------------------------------

OFStream& operator << (OFStream& file, const TplFieldDataFix& tpl) ffThrowAll
{
    file << uns1_t(0)   //version
         << uns4_t( (tpl.bOptional   ? (1 << 0) : 0) )
         << tpl.nSize;

    return file;
}

IFStream& operator >> (IFStream& file, TplFieldDataFix& tpl) throw()
{
    uns1_t nVer; file >> nVer;
    CheckVersion(nVer, 0);
    
    uns4_t nFlags;    
    file >> nFlags
         >> tpl.nSize;

    tpl.bOptional   = 0 != (nFlags & (1 << 0));
         
    return file;
}

//----------------------------------------------------------------------------

OFStream& operator << (OFStream& file, const TplFieldDataVar& tpl) ffThrowAll
{
    file << uns1_t(0)   //version
         << uns4_t( (tpl.bZeroTerminated ? (1 << 0) : 0) | 
                    (tpl.bStoreZero      ? (1 << 1) : 0) );
         
    return file;
}

IFStream& operator >> (IFStream& file, TplFieldDataVar& tpl) throw()
{
    uns1_t nVer; file >> nVer;
    CheckVersion(nVer, 0);
    
    uns4_t nFlags; file >> nFlags;

    tpl.bZeroTerminated = 0 != (nFlags & (1 << 0));
    tpl.bStoreZero      = 0 != (nFlags & (1 << 1));
         
    return file;
}

//----------------------------------------------------------------------------

OFStream& operator << (OFStream& file, const TplFieldSearchText& tpl) ffThrowAll
{
    file << uns1_t(0)   //version
         << uns1_t(tpl.titType)
         << uns4_t( (tpl.bTextCaseSensitive ? (1 << 0) : 0) |
                    (tpl.bTextWithControls  ? (1 << 1) : 0) )
         << tpl.nWordLenMin
         << tpl.nWordLenMax
         << tpl.nStopWordsCount
         << string(tpl.pszDelimiters);
         
    for (size_t n = 0; n < tpl.nStopWordsCount; ++n)
        file << string(tpl.arrStopWords[n]);        
         
    return file;    
}

void LoadTpl(IFStream& file, TplFieldSearchText& tpl, TplFieldSearchTextContext& ctx) throw()
{
    uns1_t nVer; file >> nVer;
    CheckVersion(nVer, 0);
    
    uns1_t nType; file >> nType;
    tpl.titType = TextIndexType(nType);
    uns4_t nFlags; 
    file >> nFlags
         >> tpl.nWordLenMin
         >> tpl.nWordLenMax
         >> tpl.nStopWordsCount;

    tpl.bTextCaseSensitive = 0 != (nFlags & (1 << 0));
    tpl.bTextWithControls  = 0 != (nFlags & (1 << 1));
    
    ctx.m_vecStr.resize(1 + tpl.nStopWordsCount);
    file >> ctx.m_vecStr.front();
    tpl.pszDelimiters = ctx.m_vecStr.front().c_str();
    
    ctx.m_vecPtr.resize(tpl.nStopWordsCount);
    for (size_t n = 0; n < tpl.nStopWordsCount; ++n)
    {
        file >> ctx.m_vecStr[1 + n];
        ctx.m_vecPtr[n] = ctx.m_vecStr[1 + n].c_str();
    }
    
    tpl.arrStopWords = (0 == tpl.nStopWordsCount) ? 0 : &*ctx.m_vecPtr.begin();
}

//----------------------------------------------------------------------------

OFStream& operator << (OFStream& file, const TplIndex& tpl) ffThrowAll
{
    file << uns1_t(0)   //version
         << tpl.nRanksCount
         << tpl.nFieldDataFixCount
         << tpl.nFieldDataVarCount
         << tpl.nFieldSearchTextCount;

    for (size_t n = 0; n < tpl.nFieldDataFixCount; ++n)
        file << tpl.arrFieldDataFix[n];

    for (size_t n = 0; n < tpl.nFieldDataVarCount; ++n)
        file << tpl.arrFieldDataVar[n];

    for (size_t n = 0; n < tpl.nFieldSearchTextCount; ++n)
        file << tpl.arrFieldSearchText[n];    

    return file;
}

void TplIndexLoader::Load(IFStream& file) throw()
{
    uns1_t nVer; file >> nVer;
    CheckVersion(nVer, 0);
    file >> nRanksCount
         >> nFieldDataFixCount
         >> nFieldDataVarCount
         >> nFieldSearchTextCount;
         
    m_vecDataFix.resize(nFieldDataFixCount);
    for (size_t n = 0; n < nFieldDataFixCount; ++n)
        file >> m_vecDataFix[n];
    arrFieldDataFix = (0 == nFieldDataFixCount) ? 0 : &*m_vecDataFix.begin();        

    m_vecDataVar.resize(nFieldDataVarCount);
    for (size_t n = 0; n < nFieldDataVarCount; ++n)
        file >> m_vecDataVar[n];
    arrFieldDataVar = (0 == nFieldDataVarCount) ? 0 : &*m_vecDataVar.begin();                

    m_vecSearchText.resize(nFieldSearchTextCount);
    m_vecSearchTextContext.resize(nFieldSearchTextCount);
    for (size_t n = 0; n < nFieldSearchTextCount; ++n)
    {
        SetBit(arrFieldAS, nFieldDataFixCount + nFieldDataVarCount + n);
        LoadTpl(file, m_vecSearchText[n], m_vecSearchTextContext[n]);
    }        
    arrFieldSearchText = (0 == nFieldSearchTextCount) ? 0 : &m_vecSearchText[0];                    
}

}//namespace FastFish

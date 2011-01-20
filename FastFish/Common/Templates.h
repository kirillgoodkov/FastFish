#pragma once
#include "Common/Common.h"

namespace FastFish {

struct TplFieldDataFix
{
    datasize_t      nSize;
    bool            bOptional;

    TplFieldDataFix()                                   throw();
    bool operator == (const TplFieldDataFix& oth) const throw();        
};

struct TplFieldDataVar
{
    bool            bZeroTerminated;    //treat pData in AddField as zero terminated string, ignore nSize argument
    bool            bStoreZero;         //store terminate '\0'. it's allows to use the pointer returned by GetData, as C-style string

    TplFieldDataVar()                                   throw();
    bool operator == (const TplFieldDataVar& oth) const throw();
};

struct TplFieldSearchText
{
    TextIndexType   titType;
    const char*     pszDelimiters;      //only ASCII subset of UTF8
    wordsize_t      nWordLenMin;        //in UTF8 symbols
    wordsize_t      nWordLenMax;        //in bytes
    uns4_t          nStopWordsCount;
    const char**    arrStopWords;
    bool            bTextCaseSensitive;
    bool            bTextWithControls;  //text may contain symbols lower than '\x20', and these symbols not a delimiters

    TplFieldSearchText()                                   throw();    
    bool operator == (const TplFieldSearchText& oth) const throw();
};

struct TplIndex
{
    rank_t                      nRanksCount;    
    fldid_t                     nFieldDataFixCount;
    const TplFieldDataFix*      arrFieldDataFix;
    fldid_t                     nFieldDataVarCount;
    const TplFieldDataVar*      arrFieldDataVar;
    fldid_t                     nFieldSearchTextCount;
    const TplFieldSearchText*   arrFieldSearchText;    

    TplIndex()                                     throw();    
    bool operator == (const TplIndex& oth)   const throw();    
    fldid_t FieldsCount()                    const throw()  {return nFieldDataFixCount + nFieldDataVarCount + nFieldSearchTextCount;}
};

}//namespace FastFish 

#include "Common/Templates.inl"

namespace FastFish{

inline
TplFieldDataFix::TplFieldDataFix() throw(): 
    nSize(1), 
    bOptional(false) 
{};

inline
bool TplFieldDataFix::operator == (const TplFieldDataFix& oth) const throw()
{
    return nSize     == oth.nSize     && 
           bOptional == oth.bOptional ;
}

//----------------------------------------------------------------------------

inline
TplFieldDataVar::TplFieldDataVar() throw(): 
    bZeroTerminated(false), 
    bStoreZero(false)
{};    

inline
bool TplFieldDataVar::operator == (const TplFieldDataVar& oth) const throw()
{
    return bZeroTerminated == oth.bZeroTerminated &&
           bStoreZero      == oth.bStoreZero      ;
}

//----------------------------------------------------------------------------

inline
TplFieldSearchText::TplFieldSearchText() throw(): 
    titType(titExact), 
    pszDelimiters(" \t\n\r"), 
    nWordLenMin(1), 
    nWordLenMax(MaxWordSize), 
    nStopWordsCount(0), arrStopWords(0), 
    bTextCaseSensitive(false), 
    bTextWithControls(false)
{
}

inline
bool TplFieldSearchText::operator == (const TplFieldSearchText& oth) const throw()
{
    bool bEqual = titType            == oth.titType             &&
                  0 == strcmp(pszDelimiters, oth.pszDelimiters) &&
                  nWordLenMin        == oth.nWordLenMin         &&
                  nWordLenMax        == oth.nWordLenMax         &&
                  nStopWordsCount    == oth.nStopWordsCount     &&
                  bTextCaseSensitive == oth.bTextCaseSensitive  &&
                  bTextWithControls  == oth.bTextWithControls   ;
                  
    for (size_t n = 0; n < nStopWordsCount; ++n)
    {
        bEqual = bEqual && 0 == strcmp(arrStopWords[n], oth.arrStopWords[n]);
    }        

    return bEqual;                  
}

//----------------------------------------------------------------------------

inline
TplIndex::TplIndex() throw(): 
    nRanksCount(1), 
    nFieldDataFixCount(0),  
    arrFieldDataFix(0), 
    nFieldDataVarCount(0),  
    arrFieldDataVar(0), 
    nFieldSearchTextCount(0), 
    arrFieldSearchText(0) 
{}

inline
bool TplIndex::operator == (const TplIndex& oth) const throw()
{
    bool bEqual = nRanksCount           == oth.nRanksCount        &&
                  nFieldDataFixCount    == oth.nFieldDataFixCount && 
                  nFieldDataVarCount    == oth.nFieldDataVarCount && 
                  nFieldSearchTextCount == oth.nFieldSearchTextCount;
    
    for (size_t n = 0; n < nFieldDataFixCount; ++n)
        bEqual = bEqual && arrFieldDataFix[n] == oth.arrFieldDataFix[n];

    for (size_t n = 0; n < nFieldDataVarCount; ++n)
        bEqual = bEqual && arrFieldDataVar[n] == oth.arrFieldDataVar[n];

    for (size_t n = 0; n < nFieldSearchTextCount; ++n)
        bEqual = bEqual && arrFieldSearchText[n] == oth.arrFieldSearchText[n];    

    return bEqual;
}            

}//namespace FastFish

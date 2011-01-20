#include "../FastFish/FastFish.h"

#ifdef _MSC_VER
    #define _CRT_DISABLE_PERFCRIT_LOCKS 
    #define fflush _fflush_nolock
#elif defined(__GNUG__)
    #ifdef __USE_MISC
        #define fgets fgets_unlocked
        #define fflush fflush_unlocked
    #endif        
#endif    
    
#include <cstdio>
#include <cmath>
#include <ctime>

using namespace std;

/////////////////////////////////////////////////////////////////////////////

const char g_pszHelp[] = "\
GeoImport [ARGS] [NAME]                                 \n\
    ARGS:                                    (default)  \n\
        -s[NUM]     split                     false     \n\
                    NUM: records count        ~130000   \n\
        -sm[NUM]    split by out of memory    false     \n\
                    NUM: bubble size (in MB)  300       \n\
        -t(e|p|i)   type: exact/prefix/infix  p         \n\
    NAME (default): index                               \n\
    output file: NAME.TYPE.CONFIG.PLATFORM[.sSEGNUM]";

const char g_pszInputFile[] = "allCountries.txt";    


const double    MaxPopulation    = 3800000000.;
const unsigned  PopulationCut    = 100;
const unsigned  DefaultSplitVar  = 200;

enum SplitType
{
    stNone,
    stByRec,
    stByRecFix,
    stByMem
};

SplitType   g_SplitType    = stNone;

string      g_strName      = "index";
unsigned    g_nSplitSize   = 130000;
unsigned    g_nSplitBubble = 300;

unsigned    g_nSegment     = 0;
char*       g_pBubble      = 0;

/////////////////////////////////////////////////////////////////////////////

FastFish::Build::Builder*       g_pBuilder = 0;
FastFish::TplFieldDataFix       g_arrTplFix[2];
FastFish::TplFieldDataVar       g_tplVar;
FastFish::TplFieldSearchText    g_tplTxt;
FastFish::TplIndex            g_tpl;

/////////////////////////////////////////////////////////////////////////////

void __cdecl OutOfMemoryHandler() throw()
{
    if (g_SplitType != stByMem)
    {
        puts("out of memory");
        exit(4);
    }
        
    if (0 == g_pBubble)
    {
        puts("out of memory; bubble is to small");
        exit(5);
        
    }    
        
    delete [] g_pBubble; g_pBubble = 0;                    
}

inline
FastFish::rank_t Population2Rank(unsigned nPop)
{
    const double dConvLog = 1. / log(double(MaxPopulation)/PopulationCut);

    FastFish::rank_t rank;
    if (0 == nPop)
    {
        rank = 0;
    }
    else if (nPop < PopulationCut)
    {
        rank = 1;
    }
    else
    {
        double dLog = log(double(nPop)/PopulationCut) * dConvLog;
        dLog = min(dLog, 1.);
        rank = 1 + FastFish::rank_t((g_tpl.nRanksCount - 2)*dLog);
    }
    return rank;
}

inline
bool Match(char*& psz, const char* pszWhat)
{
    size_t nLen  = strlen(psz);
    size_t nLenW = strlen(pszWhat);
    if (nLenW <= nLen && 0 == memcmp(psz, pszWhat, nLenW))
    {
        psz += nLenW;
        return true;
    }
    return false;
}

void Store()
{
    printf("\nFinalize: ...");
    fflush(stdout);
    g_pBuilder->Finalize();
    printf("\rFinalize: done\n");

    string strName(g_strName);    
    switch (g_tplTxt.titType)
    {
        case FastFish::titExact:  strName += ".exc"; break;
        case FastFish::titPrefix: strName += ".prf"; break;
        case FastFish::titInfix:  strName += ".inf"; break;
    }    
    strName += ffDbgOrRel(".dbg", ".rel");
    strName += ff32Or64(".32", ".64");
    if (g_SplitType != stNone)
    {
        char chExt[12];
        sprintf(chExt, ".s%02u", g_nSegment++);
        strName += chExt;        
    }
    
    printf("Store %s: ...", strName.c_str());
    g_pBuilder->Store(strName.c_str());
    printf("\rStore %s: done\n\n", strName.c_str());
    fflush(stdout);

    delete g_pBuilder; g_pBuilder = 0;
    
    if (g_SplitType == stByRec)
    {
        g_nSplitSize += (rand()%DefaultSplitVar) - DefaultSplitVar/2;
    }        
}

inline
char* NextCol(char* psz)
{
    while('\t' != *psz++) 
    {}            
    return psz;
}

inline
char* MarkEnd(char* psz)
{
    while('\t' != *psz) 
    {
        ++psz;
    }
    *psz = 0;
    return psz;
}

inline
unsigned StrToUns(const char* _psz) throw()
{
    const unsigned char* psz = reinterpret_cast<const unsigned char*>(_psz);    
    unsigned n = 0;     
    for(;;)
    {
        unsigned nDig = *psz++ - '0';
        if (nDig < 10)
            n = 10*n + nDig;
        else
            break;
    }
    return n;
}


void Import(FILE* pFile)
{
    if (g_SplitType == stByMem)
        g_pBubble = new char[g_nSplitBubble << 20];
    g_pBuilder = new FastFish::Build::Builder(g_tpl);

    enum Fields {fldDfId, fldDfPopulation, fldDvName, fldTxNames};            
    static char szBuff[25001];
    
    for (unsigned nLineTot = 0, nLine = 0; 
         fgets(szBuff, ffCountOf(szBuff) - 1, pFile);
         ++nLineTot, ++nLine)
    {    
        unsigned nId = StrToUns(szBuff);
        g_pBuilder->AddField(fldDfId, &nId);
        
        char* psz    = NextCol(szBuff);
        char* pszEnd = MarkEnd(psz);
        g_pBuilder->AddField(fldDvName, psz);
        g_pBuilder->AddField(fldTxNames, psz);        
        
        psz    = NextCol(pszEnd + 1);        
        pszEnd = MarkEnd(psz);
        g_pBuilder->AddField(fldTxNames, psz);
        
        psz = NextCol(pszEnd + 1);
        for (size_t n = 0; n < 9; ++n)
            psz = NextCol(psz);        
        unsigned nPop = StrToUns(psz);
        if (0 != nPop)
            g_pBuilder->AddField(fldDfPopulation, &nPop);
            
        g_pBuilder->CommitDocument(Population2Rank(nPop));                                
        
        if (0 == (nLineTot&0x7FFF))
        {
            printf("Read %s: %u%%\r", g_pszInputFile, nLineTot/73000);                    
            fflush(stdout);
        }            

        if (( g_SplitType == stByMem                               && 0 == g_pBubble) || 
            ((g_SplitType == stByRec || g_SplitType == stByRecFix) && g_nSplitSize == nLine))
        {
            Store();
            if (g_SplitType == stByMem)
                g_pBubble = new char[g_nSplitBubble << 20];                        
            g_pBuilder   = new FastFish::Build::Builder(g_tpl);        
            nLine        = 0;
        }                
    }
            
    printf("Read %s: done\n", g_pszInputFile);
    fflush(stdout);
    
    delete [] g_pBubble; g_pBubble = 0;
    Store();      
}

int __cdecl main(int argc, char* argv[])
{
    set_new_handler(OutOfMemoryHandler);
    FastFish::SetLog(stdout);
    
    g_arrTplFix[0].nSize     = 4;
    g_arrTplFix[0].bOptional = false;
    g_arrTplFix[1].nSize     = 4;
    g_arrTplFix[1].bOptional = true;    
    g_tplVar.bZeroTerminated = true;    
    g_tplVar.bStoreZero      = true;    
    g_tplTxt.titType            = FastFish::titPrefix;
    g_tplTxt.pszDelimiters      = " ,()/\\-;\t.\"'";
    g_tplTxt.bTextCaseSensitive = false;
    g_tplTxt.bTextWithControls  = false;    
    g_tpl.nRanksCount           = 254;
    g_tpl.nFieldDataFixCount    = ffCountOf(g_arrTplFix);
    g_tpl.arrFieldDataFix       = g_arrTplFix;
    g_tpl.nFieldDataVarCount    = 1;
    g_tpl.arrFieldDataVar       = &g_tplVar;
    g_tpl.nFieldSearchTextCount = 1;
    g_tpl.arrFieldSearchText    = &g_tplTxt;
    
    for (int n = 1; n < argc; ++n)
    {
        char* psz = argv[n];
        if (Match(psz, "-"))
        {
            if (Match(psz, "sm"))
            {
                g_SplitType = stByMem;
                if (isdigit(*psz))
                {
                    g_nSplitBubble = atoi(psz);
                }
            }
            else if (Match(psz, "s"))
            {
                if (isdigit(*psz))
                {
                    g_SplitType  = stByRecFix;
                    g_nSplitSize = atoi(psz);
                }
                else
                {
                    g_SplitType  = stByRec;
                }
            }
            else if (Match(psz, "te"))
            {
                g_tplTxt.titType = FastFish::titExact;
            }
            else if (Match(psz, "tp"))
            {
                g_tplTxt.titType = FastFish::titPrefix;
            }
            else if (Match(psz, "ti"))
            {
                g_tplTxt.titType = FastFish::titInfix;
            }
            else
            {
                printf("invalid argument %s\n%s\n", psz, g_pszHelp);
                return 1;
            }        
        }
        else
        {
            g_strName = psz;
        }            
    }
    
    FILE* pFile = fopen(g_pszInputFile, "rb");
    if (0 == pFile)
    {
        printf("%s not found\n", g_pszInputFile);
        return 2;
    }

    try
    {
        clock_t ctStart = clock();
        Import(pFile);                  
        printf("Time elapsed: %7.3f\n", double(clock() - ctStart)/CLOCKS_PER_SEC);
    }
    catch(const exception& ex)
    {
        puts(ex.what());
        return 3;
    }

    fclose(pFile);
        
    return 0;
}


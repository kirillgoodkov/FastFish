#include "../FastFish/FastFish.h"

#ifdef _MSC_VER
    #define _CRT_DISABLE_PERFCRIT_LOCKS
    #define ftell _ftelli64_nolock
    #define fflush _fflush_nolock
#elif defined(__GNUG__)
    #ifdef __USE_MISC
        #define fgets fgets_unlocked
        #define fflush fflush_unlocked
    #endif        
    #ifdef _COMPILING_NEWLIB
        #define ftell ftello64
    #else
        #define ftell ftello                
    #endif    
#endif    

#include <cstdio>
#include <string>
#include <ctime>

using namespace std;

/////////////////////////////////////////////////////////////////////////////

const char g_pszHelp[] = "\
WikiImport [ARGS] [NAME]                                 \n\
    ARGS:                                    (default)  \n\
        -s[NUM]     split                     false     \n\
                    NUM: records count        1000000   \n\
        -sm[NUM]    split by out of memory    false     \n\
                    NUM: bubble size (in MB)  600       \n\
        -t(e|p|i)   type: exact/prefix/infix  p         \n\
    NAME (default): index                               \n\
    output file: NAME.TYPE.CONFIG.PLATFORM[.sSEGNUM]";

const char g_pszInputFile[] = "enwiki-latest-pages-articles.xml";

enum SplitType
{
    stNone,
    stByRec,
    stByMem
};

SplitType           g_SplitType    = stNone;

string              g_strName      = "index";
unsigned            g_nSplitSize   = 1000000;
unsigned            g_nSplitBubble = 600;

unsigned            g_nSegment     = 0;
char*               g_pBubble      = 0;
unsigned long long  g_fposPrev     = 0;
unsigned long long  g_fpos         = 0;
clock_t             g_ctSeg        = 0;        

const char* g_arrStopWords[] = {"text", "title", "quot", "category", "links", "references", "http", "external", "image", "thumb", "html", "shtml", "nbsp", "location",};

//////////////////////////////////////////////////////////////////////////////

FastFish::Build::Builder*       g_pBuilder = 0;

FastFish::TplFieldDataVar       g_tplVar;
FastFish::TplFieldSearchText    g_tplTxt;
FastFish::TplIndex            g_tpl;

//////////////////////////////////////////////////////////////////////////////

void __cdecl OutOfMemoryHandler() throw()
{
    if (g_SplitType != stByMem)
    {
        printf("\nout of memory\n");
        exit(4);
    }
        
    if (0 == g_pBubble)
    {
        printf("\nout of memory; bubble is to small");
        exit(5);
        
    }    
        
    delete [] g_pBubble; g_pBubble = 0;                    
}

bool Find(char*& psz, const char* pszString)
{
    psz = strstr(psz, pszString);
    if (0 != psz && 0 != *psz)
    {
        psz += strlen(pszString);
        return true;
    }
    return false;
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
    fflush(stdout);

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
        sprintf(chExt, ".s%03u", g_nSegment++);
        strName += chExt;        
    }
    
    printf("Store %s: ...", strName.c_str()); 
    fflush(stdout);
    g_pBuilder->Store(strName.c_str());
    printf("\rStore %s: done, time elapsed: %7.3f\n\n", strName.c_str(), double(clock() - g_ctSeg)/CLOCKS_PER_SEC); 
    fflush(stdout);
    g_ctSeg = clock();

    delete g_pBuilder; g_pBuilder = 0;    
    g_fposPrev = g_fpos;    
}

void ScanLine(char* psz)
{
    enum State
    {
        stRoot,
        stPage,
        stPageNop,
        stRevision,
        stText,
        stTextCut
    };

    static State st         = stRoot;
    static string strTitle;
    static size_t nCutDeep  = 0;
    
    while (psz && *psz)
    {
        switch (st)
        {
            case stRoot:
                if (Find(psz, "<page>"))
                    st = stPage;
                break;
            case stPage:
                if (Find(psz, "<"))
                {
                    switch (*psz)
                    {
                        case 'r':
                            if (Match(psz, "redirect />"))
                                st = stPageNop;
                            else if (Match(psz, "revision>"))
                                st = stRevision;
                            break;
                        case 't':
                            if (Match(psz, "title>"))
                            {
                                strTitle.assign(psz, strstr(psz, "</title>"));                                
                                return;
                            }
                            break;
                        case '/':
                            if (Match(psz, "/page"))
                                st = stRoot;
                            break;
                        default:
                            Find(psz, ">");
                    }
                }
                break;
            case stPageNop:
                if (Find(psz, "</page>"))
                    st = stRoot;
                break;
            case stRevision:
                if (Find(psz, "<"))
                {
                    if (Match(psz, "text"))
                    {
                        Find(psz, ">");
                        st = stText;
                    }
                    else if (Match(psz, "/revision"))
                    {
                        st = stPage;
                    }                    
                }                   
                    
                break;
            case stText:
            {
                char* pszOld = psz;
                if (Find(psz, "{{"))
                {
                    psz[-2] = 0;
                    g_pBuilder->AddField(1, pszOld);
                    st = stTextCut;
                    ++nCutDeep;
                }
                else
                {
                    g_pBuilder->AddField(1, pszOld);
                    if (Find(pszOld, "</text>"))
                    {
                        psz = pszOld;                        
                        g_pBuilder->AddField(0, strTitle.c_str());
                        g_pBuilder->CommitDocument();
                        st = stRevision;
                        if (stByRec == g_SplitType)
                        {
                            if (g_nSplitSize <= g_pBuilder->DocumentsCount())
                            {
                                Store();
                                g_pBuilder = new FastFish::Build::Builder(g_tpl);
                            }
                        }
                        else if (stByMem == g_SplitType)
                        {
                            if (0 == g_pBubble)
                            {
                                Store();
                                g_pBubble  = new char[g_nSplitBubble << 20];                                
                                g_pBuilder = new FastFish::Build::Builder(g_tpl);
                            }
                        }
                    }
                }               
                break;
            }                
            case stTextCut:
            {
                char* pszOpen = strstr(psz, "{{");
                char* pszClose = strstr(psz, "}}");
                
                if (pszOpen && pszClose)
                {
                    if (pszOpen < pszClose)
                        pszClose = 0;
                    else             
                        pszOpen = 0;           
                }                        
                
                if (pszOpen)
                {
                    psz = pszOpen + 2;
                    ++nCutDeep;
                }
                else if (pszClose)
                {
                    psz = pszClose + 2;
                    --nCutDeep;
                    if (0 == nCutDeep)
                        st = stText;
                }
                else
                {
                    goto lBreak;
                }

                break;
            }                
        }
    }

    lBreak:
    if (stTextCut == st)    
    {
        st = stText;
        nCutDeep = 0;
    }
    
}

void Import(FILE* pFile)
{
    if (g_SplitType == stByMem)
        g_pBubble = new char[g_nSplitBubble << 20];
    g_pBuilder = new FastFish::Build::Builder(g_tpl);

    static char chBuff[0x10000];           
    
    if (0 == g_pBuilder)
        g_pBuilder = new FastFish::Build::Builder(g_tpl);

    for (unsigned nLine = 0; fgets(chBuff, ffCountOf(chBuff), pFile); ++nLine)           
    {
        ScanLine(chBuff); 
        
        if (0 == (nLine & 0x3FFF))
        {   
            g_fpos = ftell(pFile);
            printf("Lines: %10u  MBytes: %4u Documents: %u\r", nLine, unsigned((g_fpos - g_fposPrev)/(1 << 20)), g_pBuilder->DocumentsCount()); 
            fflush(stdout);
        }
    }        
    Store();            
    printf("Read %s: done\n", g_pszInputFile);
    fflush(stdout);
}


int main(int argc, char* argv[])
{
    std::set_new_handler(OutOfMemoryHandler);    
    
    g_tplVar.bZeroTerminated = true;
    g_tplVar.bStoreZero      = true;
    g_tplTxt.titType            = FastFish::titPrefix;
    g_tplTxt.pszDelimiters      = " !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
    g_tplTxt.nWordLenMin        = 3;
    g_tplTxt.nWordLenMax        = 100; 
    g_tplTxt.nStopWordsCount    = ffCountOf(g_arrStopWords); 
    g_tplTxt.arrStopWords       = g_arrStopWords; 
    g_tplTxt.bTextCaseSensitive = false; 
    g_tplTxt.bTextWithControls  = false; 
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
                g_SplitType  = stByRec;
                if (isdigit(*psz))
                {
                    g_nSplitSize = atoi(psz);
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
    
    FILE* pFile = fopen(g_pszInputFile, "rbS");
    if (0 == pFile)
    {
        printf("%s not found\n", g_pszInputFile);
        return 2;
    }
    
    setvbuf(pFile, 0, _IOFBF, 0x10000);

    try
    {
        FastFish::SetLog(stdout);
        clock_t ctStart = clock();        
        g_ctSeg         = ctStart;
        Import(pFile);                    
        printf("Total time elapsed: %7.3f\n", double(clock() - ctStart)/CLOCKS_PER_SEC);
        fflush(stdout);
    }
    catch(const exception& ex)
    {
        printf("%s", ex.what());
        return 3;
    }

    fclose(pFile);
        
    return 0;
}


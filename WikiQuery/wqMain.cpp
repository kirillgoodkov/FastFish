#include "../FastFish/FastFish.h"

#ifdef _MSC_VER
    #define RN "\n"
    #define _CRT_DISABLE_PERFCRIT_LOCKS
    #define fflush _fflush_nolock
#elif defined(__GNUG__)
    #define RN "\r\n"
    #ifdef __USE_MISC
        #define fgets fgets_unlocked
        #define fflush fflush_unlocked
    #endif        
#endif    

#include <list>
#include <cstdio>
#include <ctime>
#include <io.h>

using namespace std;

/////////////////////////////////////////////////////////////////////////////

const char g_pszHelp[] = "\
WikiQuery [ARGS] FILENAME ...                                       \n\
    ARGS:                                              (default)    \n\
        -p[NUM]     prefetch data                       true        \n\
                    NUM: limit(MB), 0 - skip prefetch   inf         \n\
        -l[NUM]     lock critical data                  true        \n\
                    NUM: limit(MB), 0 - skip locking    inf         \n\
        -q(a|s)     query type - all|some               a           \n\
        -quNUM\"Q\" user query, NUM - arg count(1-4)                \n\
        -rNUM       restrict output (item count)        10          \n\
        -rn         don't restrict output               false       \n\
        -v          verbose output                                  \n\
    FILENAME: index file name";


const unsigned  g_nMaxArgs      = 4;    
bool            g_bVerbose      = false;
size_t          g_nOutputLimit  = 10;  
size_t          g_nPrefetch     = size_t(-1);
size_t          g_nLock         = size_t(-1);
string          g_strQuery      = "a1,0";
unsigned        g_nArgs         = 1;
list<string>    g_lstMasks;

/////////////////////////////////////////////////////////////////////////////

FastFish::Search::Searcher g_Searcher;           

/////////////////////////////////////////////////////////////////////////////


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

void RunQueries()
{
    clock_t ctStart = clock();
    
    FastFish::Search::Query* q = g_Searcher.CreateQuery(g_strQuery.c_str());
    
    char arrArgs[g_nMaxArgs][0x400];
    for (unsigned nQ = 0; fgets(arrArgs[0], ffCountOf(arrArgs[0]), stdin); ++nQ)
    {
        const unsigned nInterval = 1000;
        if (0 == nQ%nInterval && 0 != nQ)
        {
            static clock_t ctOld = 0;
            clock_t ct = clock();        
            if (1 < ct - ctOld)
            {
                fprintf(stderr, "queries: %7u; time: %9.3f; q/s: %8.1f\n", 
                            nQ, 
                            double(ct - ctStart)/CLOCKS_PER_SEC, 
                            nInterval*CLOCKS_PER_SEC/double(ct - ctOld));
                fflush(stderr);
            }
            ctOld = ct;                        
        }
        
        for (size_t n = 1; n < g_nArgs; ++n)
        {
            fgets(arrArgs[n], ffCountOf(arrArgs[n]), stdin);
        }
        FastFish::Search::Sequence* s = q->CreateSequence(g_nArgs, arrArgs[0], arrArgs[1], arrArgs[2], arrArgs[3]);
       
        if (s)
        {
            printf("%s", arrArgs[0]);
            size_t nCount = 0;                
            for (s->First(); !s->IsFinished() && nCount < g_nOutputLimit; s->Next(), ++nCount) 
            {
                FastFish::Document doc(*s); 
                
                if (g_bVerbose)
                {
                    printf("did:%7u\tsid:%u\trank:%5u\t", unsigned(doc.m.did), unsigned(doc.m.sid), unsigned(doc.m.rank));
                }                       
                    
                printf("%s" RN, (const char*)g_Searcher.GetData(0, doc));
            }                        
            
            printf("%s:\t%u (%u)" RN RN, 
                    (g_nOutputLimit == nCount) ? "limit" : "total", 
                    unsigned(nCount), 
                    unsigned(s->GetApproximateLength()));
            
            q->DeleteSequence(s);
        }        
    }            
    g_Searcher.DeleteQuery(q);
}

int __cdecl main(int argc, char* argv[])
{
    for (int n = 1; n < argc; ++n)
    {
        char* psz = argv[n];
        if (Match(psz, "-"))
        {
            if (Match(psz, "l"))
            {
                g_nLock = isdigit(*psz) ? (size_t(atoi(psz)) << 20) : size_t(-1);
            }
            else if (Match(psz, "p"))
            {
                g_nPrefetch = isdigit(*psz) ? (size_t(atoi(psz)) << 20) : size_t(-1);
            }
            else if (Match(psz, "qa"))
            {
                g_strQuery = "a1,0";
            }
            else if (Match(psz, "qs"))
            {
                g_strQuery = "s1,0";
            }
            else if (Match(psz, "qu"))
            {
                g_nArgs = min(unsigned(atoi(psz)), g_nMaxArgs);
                while(isdigit(*psz))
                    ++psz;
                g_strQuery = psz;
            }
            else if (Match(psz, "v"))
            {
                g_bVerbose = true;
            }
            else if (Match(psz, "rn"))
            {
                g_nOutputLimit = size_t(-1);                
            }
            else if (Match(psz, "r") && isdigit(*psz))
            {
                g_nOutputLimit = atoi(psz);
            }
            else
            {
                fprintf(stderr, "invalid argument %s\n%s\n", psz, g_pszHelp);
                return 1;
            }        
        }
        else
        {
            g_lstMasks.push_back(psz);
        }            
    }
    if (g_lstMasks.empty())
    {
        fprintf(stderr, "index(es) not specified\n%s\n", g_pszHelp);
        return 1;        
    }
    
    try
    {                                 
        clock_t ctStart = clock();        
        
        FastFish::SetLog(stdout, FastFish::LogStream(FastFish::lsInfo | FastFish::lsWarn));
        for (list<string>::const_iterator it = g_lstMasks.begin(); it != g_lstMasks.end(); ++it)
        {
            #ifdef _MSC_VER        
                _finddata_t fd;
                intptr_t hFind = _findfirst(it->c_str(), &fd);            
                if(-1 == hFind)
                {
                    fprintf(stderr, "No %s file(s) in current directory\n", it->c_str());
                    return 2;
                }
                else
                {
                    do 
                    {
                        fprintf(stderr, "load %s\n", fd.name);
                        fflush(stderr);
                        g_Searcher.Load(fd.name);                    
                    } 
                    while(0 == _findnext(hFind, &fd));
                    _findclose(hFind);
                }
            #else
                fprintf(stderr, "load %s\n", it->c_str());
                fflush(stderr);
                g_Searcher.Load(it->c_str());
            #endif                
        }        

        fprintf(stderr, "loading done\n");
        fflush(stderr);
            
        if (g_nLock)            
        {
            bool b = g_Searcher.LockCritical(g_nLock);
            fprintf(stderr, "LockCritical %s\n", b ? "done" : "failed");
            fflush(stderr);
        }
        if (g_nPrefetch)
        {
            g_Searcher.Prefetch(g_nPrefetch);
            fprintf(stderr, "Prefetch done\n");
            fflush(stderr);
        }    
            
        RunQueries();
        
        fprintf(stderr, "Time elapsed: %7.3f\n", double(clock() - ctStart)/CLOCKS_PER_SEC);
        
    }
    catch(const exception& ex)
    {
        puts(ex.what());
        return 3;
    }
    
    return 0;
}


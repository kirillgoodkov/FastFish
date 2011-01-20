#pragma once

namespace FastFish {

typedef signed char             int1_t;
typedef signed short            int2_t;
typedef signed int              int4_t;
typedef signed long long        int8_t;
typedef unsigned char           uns1_t;
typedef unsigned short          uns2_t;
typedef unsigned int            uns4_t;
typedef unsigned long long      uns8_t;

typedef uns2_t                  rank_t;
typedef uns1_t                  fldid_t;
typedef uns1_t                  argid_t;
typedef uns4_t                  docid_t;
typedef uns2_t                  segid_t;
typedef uns2_t                  datasize_t;
typedef datasize_t              wordsize_t;
typedef char                    byte_t;
typedef uns8_t                  filepos_t;
typedef void*                   handle_t;
typedef uns4_t                  bmpword_t;
typedef double                  real_t;

const size_t        InvalidPos          = size_t(-1);

const size_t        MaxUns1             = uns1_t(-1);
const size_t        MaxUns2             = uns2_t(-1);
const size_t        MaxUns4             = uns4_t(-1);
const size_t        MaxSizeT            = size_t(-1);
const size_t        SizeUns1            = MaxUns1 + 1;
const uns1_t        BitsInByte          = 8;

const fldid_t       MaxFieldId          = fldid_t(-1) >> 1;
const argid_t       MaxArgumentId       = fldid_t(-1) - 1;
const docid_t       MaxDocumentId       = (docid_t(-1) >> 1) - 1;
const rank_t        MaxRank             = rank_t(-1) - 1;
const wordsize_t    MaxWordSize         = wordsize_t(-1);
const datasize_t    MaxDataSize         = datasize_t(-1);
const segid_t       MaxSegId            = segid_t(-1);
const size_t        MaxWordsInArgument  = 32;

//----------------------------------------------------------------------------

struct ExceptIO: public std::runtime_error
{
    ExceptIO(const std::string& str) throw(): std::runtime_error(str) {ffLogNN(lsExceptIO, str.c_str());}
};

struct ExceptSyntax: public std::runtime_error
{
    ExceptSyntax(const std::string& str) throw(): std::runtime_error(str) {ffLogNN(lsExceptSyntax, str.c_str());}
};

struct ExceptFileFormat: public std::runtime_error
{
    ExceptFileFormat(const std::string& str) throw(): std::runtime_error(str) {ffLogNN(lsExceptFileFormat, str.c_str());}
};


//----------------------------------------------------------------------------

enum TextIndexType
{
    titExact,
    titPrefix,
    titInfix
};

union Document
{
    struct  
    {
        #ifdef ffLittleEndian
            docid_t did;
            segid_t sid;
            rank_t  rank;
        #else
            rank_t  rank;
            segid_t sid;
            docid_t did;
        #endif
    }       m;    
    uns8_t  val;        

    Document()                        throw()   {}
    Document(uns8_t oth)              throw()   : val(oth) {}    
    operator uns8_t& ()               throw()   {return val;};
    Document& operator = (uns8_t oth) throw()   {val = oth; return *this;}
    operator const uns8_t& ()   const throw()   {return val;};    
};

}//namespace FastFish 

//----------------------------------------------------------------------------
//forward declarations

namespace FastFish
{
    struct TplIndex;
    struct TplFieldDataFix;
    struct TplFieldDataVar;
    struct TplFieldSearchText;
    
    class TplIndexLoader;    
    class OFStream;
    class IFStream;
    class MemMap;
    class MemManager;
    class AllocatorInvader;
    class AllocatorInvaderBB;
    class AllocatorSwapper;
     
    namespace Build
    {   
        struct Documents;
        struct Field;
        struct TextAllocator;
        class DocumentsNoRanks;
        class DocumentsRanked;
        template<bool bCTRL, bool bSENS, TextIndexType tit> class Node;
        template<bool bCTRL, bool bSENS, TextIndexType tit> class ReorderBFS;
    }
        
    namespace Search
    {   
        struct PsContext;
        struct PsCursor;
        struct OptNode;
        struct SeqNodeL;
        struct Field;
        struct FieldSearch;
        class Documents;
        class Query;
        class Sequence;
        class Searcher;        
    }    
}

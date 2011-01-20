#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Search{

const uns1_t AstFlagNeg     = 0x80;
const uns1_t AstMaskFlags   = 0xF0;
const uns1_t AstMaskType    = uns1_t(~AstMaskFlags);

const uns1_t AstFlagLeaf    = 0x08;
const uns1_t AstFlagSource  = 0x10;

const uns1_t InvalidArgId   = MaxArgumentId + 1;

enum AstNodeType
{                           //NodeFix:
    antNodeAnd,             //flags, left,  right
    antNodeOr,               
    antLeafA = AstFlagLeaf, //flags, fldid, arg_id
    antLeafS, 
};

struct AstNode
{
    uns1_t  nFlags;

    void SetType(AstNodeType ant) throw()   {nFlags = (nFlags & AstMaskFlags) | uns1_t(ant);}
    void FlipNeg()                throw()   {nFlags = (nFlags & AstFlagNeg) ? (nFlags & ~AstFlagNeg) : (nFlags | AstFlagNeg);}
    void FlipType()               throw()   {nFlags = (nFlags & 1) ? (nFlags & ~1) : (nFlags | 1);}

    AstNodeType GetType()   const throw()   {return AstNodeType(nFlags & AstMaskType);}
    bool IsLeaf()           const throw()   {return 0 != (nFlags & AstFlagLeaf);}
    bool IsNeg()            const throw()   {return 0 != (nFlags & AstFlagNeg);};
};

struct AstNodeFix: public AstNode
{
    uns1_t  arrArg[2];
};

struct AstNodeC: public AstNode
{
    uns1_t  nLastIdx;
    uns1_t  arrChilds[1];        
};      

struct AstLeafSop: public AstNode
{
    uns1_t nFldId;
    uns1_t nArgId;
};


}//namespace Search
}//namespace FastFish

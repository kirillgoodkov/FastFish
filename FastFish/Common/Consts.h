#pragma once
#include "Common/Common.h"

namespace FastFish{

const size_t    CacheLine   = 64;
const size_t    MinPageSize = 4096;

const real_t    TopPrefixMul = 0.005f;

namespace PC{//performance constants

    const size_t    FieldDataFixDeqShift            = 14;                      // log2
    const size_t    FieldDataVarDeqSizesShift       = 15;                      // log2
    const size_t    FieldDataVarDeqPointersShift    = 13;                      // log2
    const size_t    FieldDataVarAllocSize           = 0x10000;                 // allocator block, bytes = x
                                                  
    const size_t    FieldTextDeqSpansShift          = 10;                      // log2
    const size_t    FieldTextDeqIdxCmprShift        = 8;                       // log2
    const size_t    FieldTextDeqSuffixShift         = 9;                       // log2
    const size_t    FieldTextDeqIdxBShift           = 12;                      // log2 
    const size_t    FieldTextDeqIdxAShift           = 11;                      // log2 
                                                                                  
    const size_t    NodeInPlaceSuffixLen            = 8;                       // sizeof(void*) <= x , depend from CacheLine   
    const size_t    NodeAllocStrSize                = 0x2000;                  // allocator block, bytes
    const size_t    NodeAllocSetMul                 = 0x4000;                  // allocator block = x*SetBlockSize
                                                   
    const size_t    ReducerMapAllocSize             = 0x2000;                  // allocator block, bytes
    const size_t    ReducerStrAllocSize             = 0x1000;                  // allocator block, bytes
    const size_t    ReducerMapSkipAllocSize         = 0x800;                   // allocator block, bytes
    const size_t    ReducerSetSkipAllocSize         = 0x400;                   // allocator block, bytes
                                                   
    const size_t    SmallCharMapAllocItemMul        = 0x8000;                  // allocator block = x*sizeof(Reduce::Node)
    const size_t    SmallCharMapAllocListMul        = 0x1000;                  // allocator block = x*sizeof(CharMap::List)
    const size_t    SmallCharMapAllocMapMul         = 0x100;                   // allocator block = x*sizeof(CharMap::Map)
    const size_t    SmallCharMapListSize            = ff32Or64(6,7);           // 1 < x < 8, depend from CacheLine
    const size_t    FastCharMapListSize             = 13;                      // 32bit: 5, 9, 13, ... ; 64bit: 5, 13, 21, ...
                                                   
    const size_t    DequeRawShift                   = 12;                      // log2
    const size_t    SetBlockSize                    = CacheLine * 1;           // pow of 2, CacheLine * [1/2 - 16] , 1 - min mem 4 - max speed
                                                   
    const size_t    OFStreamBlockSize               = 0x1000;                  // in bytes
    const size_t    OFStreamBlockSizeBB             = 0x400;                   // in bytes, x < OFStreamBlockSize
    const size_t    OFStreamBufferSize              = 0x10000;                 // in bytes
                                                   
    const size_t    DocumentsDeqShift               = 6;                       // log2 
    const size_t    DocumentsAllocMul               = 0x100;                   // allocator block = x * (1 << DocumentsListDeqShift) * sizeof(docid_t)
                                                   
    const size_t    ReducerAllocMapSize             = 0x2000;                  // allocator block, bytes
    const size_t    ReducerAllocStrSize             = 0x2000;                  // allocator block, bytes
                                                   
    const size_t    SearcherPostingsAllocMul        = 0x100;                   // allocator block = x*sizeof(Postings)
    const size_t    SearcherOptimizerAllocSize      = 0x1000;                  // allocator block, bytes
                                                   
    const real_t    SearcherMergeLimit              = 7.0f;                    // 1.0 - inf
    const size_t    SearcherLongJumpBlocks          = 256;                     // 1 - inf, division between binary search and linear scan

}//namespace PF

namespace FC{//file format constants
                                                    
    const uns8_t    Signature                   = 0xF74B25081D6057EFull;
    const uns4_t    VersionMajor                = 0;
    const uns4_t    VersionMinor                = 0;
                                            
    const size_t    DataVarBlockSize            = CacheLine;
    const size_t    DataFixMaxAlign             = CacheLine;
                                            
    const uns1_t    TreeSfxInPlaceFlag          = 0x80;
    const uns1_t    TreeEmptySetFlag            = 0x40;
    const uns1_t    TreeBmpStartFrom            = 63;
    const uns1_t    TreeCountMask               = 0x3F;

    const size_t    PostingsInPlaceShift        = 3;
    const size_t    PostingsInPlace             = (1 << PostingsInPlaceShift) - 1;
    const docid_t   PostingsBmpStartFromDiv     = 11;
    const size_t    PostingsFixedListMaxSize    = 80;
    const size_t    PostingsCmprListMinShift    = 7;
    const size_t    PostingsCmprListMaxShift    = 28;
    const size_t    PostingsCmprListBlockShift  = 4;
    const docid_t   PostingsExternalFlag        = 0x80000000;
                                           
}//namespace FC


}//namespace FastFish



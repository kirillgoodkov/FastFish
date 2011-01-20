#pragma once
#include "Common/Consts.h"
#include "Common/Common.h"

namespace FastFish {

class OFStream
{
    OFStream(const OFStream&);
    OFStream& operator = (const OFStream&);
public:
    OFStream(const char* pszName)                           ffThrowAll;
    ~OFStream()                                             throw();
    
    void BagBegin(size_t nAlign = 1)                        ffThrowAll;
    void BagCommit()                                        ffThrowAll;
    void BagDiscard()                                       ffThrowAll;
    
    void Finalize()                                         ffThrowAll;

    void Write(const void* p, size_t nSize)                 ffThrowAll;    
    template<typename TYPE>    
    void WriteLowPart(TYPE n, size_t nBytes)                ffThrowAll;
    void WriteNulls(size_t nBytes)                          ffThrowAll;
    
    template<typename TYPE>
    OFStream& operator << (const TYPE& val)                 ffThrowAll;        
    template<typename TYPE>
    OFStream& operator << (const std::vector<TYPE>& vec)    ffThrowAll;

    filepos_t GetBagPos()                             const throw();
    void      DumpSize()                              const throw();

private:
    mutable filepos_t  m_nPosDS;
    
    static const size_t BlockSize   = PC::OFStreamBlockSize;
    static const size_t BlockSizeBB = PC::OFStreamBlockSizeBB;

    void NewBlock(size_t nSize = BlockSize) throw();
    void Write2File(const void* p, size_t nSize) ffThrowAll;
    void Write2Block(const void* p, size_t nSize) throw();

    FILE*       m_pFile;
    filepos_t   m_nPos;
    
    bool        m_bBag;
    filepos_t   m_nBagSize;
    
    struct Block
    {
        byte_t*     pBegin;
        size_t      nSize;

        byte_t*     pWrite;        
        
        size_t FreeSpace()   const throw() {return pWrite - pBegin;};
        size_t FilledSpace() const throw() {return nSize - FreeSpace();};
    };
    std::list<Block> m_lstBlocks;
};



}//namespace FastFish 

#include "Tools/OFStream.inl"

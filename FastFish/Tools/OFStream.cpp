#include "Tools/OFStream.h"
#include "Tools/Bits.h"

using namespace std;

namespace FastFish {

const char g_szIoError[] = "io error";

OFStream::OFStream(const char* pszName) ffThrowAll:
    m_nPosDS(0),
    m_pFile(fopen(pszName, "wb")),
    m_nPos(0),
    m_bBag(false),
    m_nBagSize(0)
{
    if (0 == m_pFile)
    {
        string strMsg("can't find/open file: ");
        strMsg.append(pszName);
        throw ExceptIO(strMsg);
    } 
    
    setvbuf(m_pFile, 0, _IOFBF, PC::OFStreamBufferSize);
    
    NewBlock();
}

OFStream::~OFStream() throw()
{
    for (list<Block>::const_iterator it = m_lstBlocks.begin(); it != m_lstBlocks.end(); ++it)    
    {
        delete [] it->pBegin;
    }
    
    if (m_pFile)
    {
        fclose(m_pFile);
    }        
}

void OFStream::BagBegin(size_t nAlign) ffThrowAll
{
    ffAssume(m_pFile);
    ffAssume(!m_bBag);
    ffAssume(nAlign <= SizeUns1);
    ffAssert(MinGreateOrEqualPow2(nAlign) == nAlign);
    
    filepos_t nMask = nAlign - 1;
    filepos_t nAdd  = (nAlign - (m_nPos & nMask)) & nMask;
    ffAssume(nAdd < nAlign);
    fpos_t nSeek = m_nPos + nAdd;
    if (fsetpos(m_pFile, &nSeek))
    {
        throw ExceptIO(g_szIoError);        
    }
    m_nPos    += nAdd;

    m_nBagSize = nAdd;    
    m_bBag     = true;
}

void OFStream::BagCommit() ffThrowAll
{
    ffAssume(m_pFile);
    ffAssume(m_bBag);
    
    Write2Block(&m_nBagSize, sizeof(m_nBagSize));
    m_nBagSize  = 0;    
    m_bBag      = false;
}

void OFStream::BagDiscard() ffThrowAll
{
    ffAssume(m_pFile);
    ffAssume(m_bBag);
    
    fpos_t nSeek = m_nPos - m_nBagSize;
    if (fsetpos(m_pFile, &nSeek))
    {
        throw ExceptIO(g_szIoError);        
    }
    m_nPos -= m_nBagSize;
    m_nBagSize = 0;
    m_bBag     = false;
}

void OFStream::Finalize() ffThrowAll
{
    ffAssume(m_pFile);
    ffAssume(!m_bBag);

    for (list<Block>::const_iterator it = m_lstBlocks.begin(); it != m_lstBlocks.end(); ++it)    
    {
        Write2File(it->pWrite, it->nSize - (it->pWrite - it->pBegin));
    }
    
    fclose(m_pFile);
    m_pFile = 0;
}

void OFStream::NewBlock(size_t nSize) throw()
{
    m_lstBlocks.push_front(Block());
    Block& block = m_lstBlocks.front();
    block.pBegin = 0;
    block.pBegin = new byte_t[nSize];
    block.nSize  = nSize;
    block.pWrite = block.pBegin + nSize;
}

void OFStream::Write2File(const void* p, size_t nSize) ffThrowAll
{
    if (0 != nSize &&
        1 != ffFWrite(p, nSize, 1, m_pFile))
    {
        throw ExceptIO(g_szIoError);
    }
    m_nPos += nSize;
}

void OFStream::Write2Block(const void* p, size_t nSize) throw()
{
    Block* pBlock = &m_lstBlocks.front();
    if (pBlock->FreeSpace() < nSize)
    {
        NewBlock(nSize < BlockSizeBB ? BlockSize : nSize);
        pBlock = &m_lstBlocks.front();            
    }
    pBlock->pWrite -= nSize;
    memcpy(pBlock->pWrite, p, nSize);                
}

void OFStream::DumpSize() const throw()
{
    ffLog(lsInfoFileSummary, "%9.3f %12X\n", (m_nPos - m_nPosDS)/double(1 << 20), m_nPos);
    m_nPosDS = m_nPos;
}



}//namespace FastFish 

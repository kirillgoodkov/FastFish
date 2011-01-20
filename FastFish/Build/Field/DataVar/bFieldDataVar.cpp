#include "Build/Field/DataVar/bFieldDataVar.h"
#include "Build/Documents/bDocumentsNoRanks.h"
#include "Build/Documents/bDocumentsRanked.h"
#include "Memory/ArrayPtr.h"
#include "Tools/OFStream.h"
#include "Tools/Bits.h"
#include "Tools/Tools.h"
#include "Common/Templates.h"

using namespace std;

namespace FastFish{
namespace Build{

template<bool bZT, bool bSZ>
FieldDataVar<bZT, bSZ>::FieldDataVar(const TplIndex& tplIndex, const TplFieldDataVar& tpl, const Documents* pDocs) throw():
    c_tplIndex(tplIndex),
    c_tpl(tpl),
    c_pDocs(pDocs),
    m_aUserData(PC::FieldDataVarAllocSize),
    m_nMaxPos(0),
    m_nMaxSize(0)
{
    ffDebugOnly(m_bAdded = false);
}

template<bool bZT, bool bSZ>
void FieldDataVar<bZT, bSZ>::Add(const void* pData, datasize_t nSize) throw()
{
    ffAssumeUser(!bZT || 0 == nSize, "nSize argument ignored for Field with bZeroTerminated flag");
    ffAssertUser(!m_bAdded, "value added twice");
    
    if (Static(bZT))
    {
        const char* psz = static_cast<const char*>(pData);
        size_t n = strlen(psz) + (Static(bSZ) ? 1 : 0);
        if (ffIsLogged(lsWarnStringCut) && MaxDataSize < n)
            ffLog(lsWarnStringCut, "string \"%.20s...\" restricted to %u symbols\n", psz, MaxDataSize);
        nSize = datasize_t(min(size_t(MaxDataSize), n));
    }

    void* p = m_aUserData.Alloc(nSize);
    
    if (Static(bZT && bSZ))
    {
        memcpy(p, pData, nSize - 1);
        static_cast<char*>(p)[nSize - 1] = 0;
    }
    else
    {
        memcpy(p, pData, nSize);
    }
    m_deqPointers.PushBack(p); 
    m_nMaxPos += nSize;
    m_nMaxSize = max(m_nMaxSize, nSize);
    m_deqSizes.PushBack(nSize);
    
    ffDebugOnly(m_bAdded = true);
}

template<bool bZT, bool bSZ>
void FieldDataVar<bZT, bSZ>::Commit(rank_t) throw()
{
    ffAssert(c_pDocs->Count() == m_deqSizes.Count() - 1);
    ffAssertUser(m_bAdded, "mandatory Field is empty");
    ffDebugOnly(m_bAdded = false);
}

template<bool bZT, bool bSZ>
template<typename POS_TYPE, typename SIZE_TYPE, typename DOCUMENTS>
void FieldDataVar<bZT, bSZ>::StoreT(OFStream& file, const DOCUMENTS* pDocs) const ffThrowAll
{
    file << uns1_t(0)//version    
         << uns1_t(sizeof(POS_TYPE)) 
         << uns1_t(sizeof(SIZE_TYPE));
         
    ArrayPtr<POS_TYPE> arrPos(m_deqPointers.Count());
    {
        file.BagBegin();                                 
        for (docid_t n = 0; n < pDocs->Count(); ++n)
        {
            docid_t did = pDocs->TransformBack(n);
            datasize_t nSize = m_deqSizes[did];
            arrPos[did] = POS_TYPE(file.GetBagPos());
            file.Write(m_deqPointers[did], nSize);
        }
        file.BagCommit();
    }
    ffLog(lsInfoFileSummary, "\tvar.data: "); file.DumpSize();

    {
        const size_t BlockSize = (FC::DataVarBlockSize - sizeof(POS_TYPE))/sizeof(SIZE_TYPE);
        struct Block
        {
            POS_TYPE    nBase;
            SIZE_TYPE   arrSizes[BlockSize];
        };
        ffAssumeStatic(FC::DataVarBlockSize == sizeof(Block));

        Block block = {POS_TYPE(-1)};
        SIZE_TYPE* pSize = block.arrSizes;
        
        file.BagBegin(sizeof(Block));
        
        for (docid_t n = 0; n < pDocs->Count(); ++n)
        {
            docid_t did = pDocs->TransformBack(n);
            *pSize = SIZE_TYPE(m_deqSizes[did]);            
            if (POS_TYPE(-1) == block.nBase)
            {
                block.nBase = arrPos[did];
            }
            
            if (++pSize == block.arrSizes + BlockSize)
            {
                file.Write(&block, sizeof(block));
                block.nBase = POS_TYPE(-1);
                pSize       = block.arrSizes;
            }        
        }
        
        if (pSize != block.arrSizes)
        {
            memset(pSize, 0, sizeof(SIZE_TYPE) * (block.arrSizes + BlockSize - pSize));
            file.Write(&block, sizeof(block));
        }        

        
        file.BagCommit();
    }    
    ffLog(lsInfoFileSummary, "\tvar.idx:  "); file.DumpSize();
}


template<bool bZT, bool bSZ>
void FieldDataVar<bZT, bSZ>::Store(OFStream& file) const ffThrowAll   
{
    if (m_nMaxPos < MaxUns4) //reserve 1 value for StoreT internal use
    {
        if (m_nMaxSize <= MaxUns1)
        {
            if (1 == c_tplIndex.nRanksCount)
                StoreT<uns4_t, uns1_t>(file, down_cast<const DocumentsNoRanks*>(c_pDocs));
            else
                StoreT<uns4_t, uns1_t>(file, down_cast<const DocumentsRanked*>(c_pDocs));                
        }
        else
        {
            if (1 == c_tplIndex.nRanksCount)
                StoreT<uns4_t, uns2_t>(file, down_cast<const DocumentsNoRanks*>(c_pDocs));
            else                
                StoreT<uns4_t, uns2_t>(file, down_cast<const DocumentsRanked*>(c_pDocs));
        }
    }
    else
    {
        if (m_nMaxSize <= MaxUns1)
        {
            if (1 == c_tplIndex.nRanksCount)
                StoreT<uns8_t, uns1_t>(file, down_cast<const DocumentsNoRanks*>(c_pDocs));
            else
                StoreT<uns8_t, uns1_t>(file, down_cast<const DocumentsRanked*>(c_pDocs));                
        }
        else
        {
            if (1 == c_tplIndex.nRanksCount)
                StoreT<uns8_t, uns2_t>(file, down_cast<const DocumentsNoRanks*>(c_pDocs));
            else
                StoreT<uns8_t, uns2_t>(file, down_cast<const DocumentsRanked*>(c_pDocs));                
        }
    }    
}

/////////////////////////////////////////////////////////////////////////////
template class FieldDataVar<false, false>;
template class FieldDataVar<true , false>;
template class FieldDataVar<true , true >;


}//namespace Build
}//namespace FastFish


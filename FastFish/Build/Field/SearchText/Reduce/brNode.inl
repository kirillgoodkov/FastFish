namespace FastFish{
namespace Build{
namespace Reduce{

template<bool bCTRL, bool bSENS> ffForceInline
bool Node<bCTRL, bSENS>::Find(const MapType& mapRoot, const char* pWord, wordsize_t nLen) throw()
{
    const Node<bCTRL, bSENS>* pNode = mapRoot.Find(*pWord);
    while (pNode)
    {
        ++pWord;
        --nLen;

        if (0 == nLen)
            return true;

        wordsize_t nEqualCount = EqualCount(pNode->m_pSuffix, pWord, std::min(nLen, pNode->m_nSuffixLen));

        if (nLen <= nEqualCount)
            return nLen == nEqualCount;

        pNode    = pNode->m_mapChilds.Find(pWord[nEqualCount]);
        pWord += nEqualCount;
        ffAssume(nEqualCount <= nLen);
        nLen     = wordsize_t(nLen - nEqualCount);
    }

    return false;
}

}//namespace Reduce
}//namespace Build
}//namespace FastFish

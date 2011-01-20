namespace FastFish{
namespace Build{

inline
docid_t DocumentsRanked::TransformForw(docid_t did) const throw()
{
    ffAssert(!m_arrIndexForw.IsEmpty());
    ffAssume(did < m_didCount);
    ffAssert(m_arrIndexForw[did] < m_didCount);
    return m_arrIndexForw[did];
}

inline
docid_t DocumentsRanked::TransformBack(docid_t did) const throw()
{
    ffAssert(!m_arrIndexBack.IsEmpty());
    ffAssume(did < m_didCount);
    ffAssert(m_arrIndexBack[did] < m_didCount);
    return m_arrIndexBack[did];
}


}//namespace Build
}//namespace FastFish

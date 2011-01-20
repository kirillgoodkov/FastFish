namespace FastFish{

template<typename TYPE> 
ArrayPtr<TYPE>::ArrayPtr() throw():
    m_pData(0)
{
    ffDebugOnly(m_nSize = 0);
}

template<typename TYPE> 
ArrayPtr<TYPE>::ArrayPtr(size_t nSize) throw():
    m_pData(new TYPE[nSize])
{
    ffDebugOnly(m_nSize = nSize);
}

template<typename TYPE> 
void ArrayPtr<TYPE>::Clear() throw()
{
    ffAssume(m_pData);
    delete [] m_pData;
    m_pData = 0;
}

template<typename TYPE> 
void ArrayPtr<TYPE>::Alloc(size_t nSize) throw()
{
    ffAssume(0 == m_pData);
    m_pData = new TYPE[nSize];
    ffDebugOnly(m_nSize = nSize);
}

template<typename TYPE> 
void ArrayPtr<TYPE>::Swap(ArrayPtr& other) throw()
{
    swap(m_pData, other.m_pData);
    ffDebugOnly(swap(m_nSize, other.m_nSize));
}

template<typename TYPE> 
TYPE& ArrayPtr<TYPE>::operator [] (size_t n) throw()
{
    ffAssume(m_pData);
    ffAssert(n < m_nSize);
    return m_pData[n];
}

template<typename TYPE> 
const TYPE& ArrayPtr<TYPE>::operator [] (size_t n) const throw()
{
    ffAssume(m_pData);
    ffAssert(n < m_nSize);
    return m_pData[n];
}

template<typename TYPE> 
TYPE* ArrayPtr<TYPE>::Get() throw()
{
    ffAssume(m_pData);
    return m_pData;    
}

template<typename TYPE> 
const TYPE* ArrayPtr<TYPE>::Get() const throw()
{
    ffAssume(m_pData);
    return m_pData;    
}

}//namespace FastFish

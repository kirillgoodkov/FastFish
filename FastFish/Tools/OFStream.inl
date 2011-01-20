namespace FastFish {

inline
filepos_t OFStream::GetBagPos() const throw()
{
    ffAssume(m_pFile);
    ffAssume(m_bBag);
    return m_nBagSize;
}     

inline
void OFStream::Write(const void* p, size_t nSize) ffThrowAll
{
    ffAssume(m_pFile);
    if (m_bBag)
    {
        Write2File(p, nSize);
        m_nBagSize += nSize;        
    }
    else
    {
        Write2Block(p, nSize);
    }
}

template<> inline
OFStream& OFStream::operator << (const bool& b) ffThrowAll
{
    uns1_t val = b ? 1 : 0;
    Write(&val, sizeof(val));
    return *this;
}

template<> inline
OFStream& OFStream::operator << (const std::string& str) ffThrowAll
{
    filepos_t nSize = str.size();
    Write(&nSize, sizeof(nSize));
    Write(str.c_str(), str.size());
    return *this;
}

template<typename TYPE> 
void OFStream::WriteLowPart(TYPE n, size_t nBytes) ffThrowAll
{
#ifdef ffLittleEndian    
    Write(&n, nBytes);
#else
    Write(reinterpret_cast<byte_t*>(&n) + sizeof(TYPE) - nBytes, nBytes);
#endif
}

#define ffDefOfsOperator(TYPE)\
template<> inline OFStream& OFStream::operator << (const TYPE& val) ffThrowAll {Write(&val, sizeof(val));return *this;}

ffDefOfsOperator(char)
ffDefOfsOperator(uns1_t)
ffDefOfsOperator(uns2_t)
ffDefOfsOperator(uns4_t)
ffDefOfsOperator(uns8_t)

inline
void OFStream::WriteNulls(size_t nBytes) ffThrowAll
{
    while (nBytes--)
    {
        *this << byte_t(0);
    }
}

template<typename TYPE> 
OFStream& OFStream::operator << (const std::vector<TYPE>& vec) ffThrowAll
{
    if (!vec.empty())
    {
        Write(&vec[0], vec.size() * sizeof(TYPE));
    }
    return *this;
}

}//namespace FastFish 

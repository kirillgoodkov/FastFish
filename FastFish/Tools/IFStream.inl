namespace FastFish{

inline
void IFStream::Read(void* p, size_t nSize) throw()
{
    m_pBack -= nSize;    
    memcpy(p, m_pBack, nSize);    
}     

#define ffDefIfsOperator(TYPE)\
template<> inline IFStream& IFStream::operator >> (TYPE& val) throw() {Read(&val, sizeof(val));return *this;}

ffDefIfsOperator(char)
ffDefIfsOperator(uns1_t)
ffDefIfsOperator(uns2_t)
ffDefIfsOperator(uns4_t)
ffDefIfsOperator(uns8_t)

template<> inline
IFStream& IFStream::operator >> (bool& b) throw()
{
    uns1_t val;
    Read(&val, sizeof(val));
    b = val == 1;
    return *this;
}

template<> inline
IFStream& IFStream::operator >> (std::string& str) throw()
{
    filepos_t nSize;
    Read(&nSize, sizeof(nSize));
    str.resize(size_t(nSize));
    Read(&str[0], size_t(nSize));
    return *this;
}

}//namespace FastFish

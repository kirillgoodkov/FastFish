namespace FastFish{
namespace Search{

ffForceInline
size_t SeqNodeDAndOr::SizeOf(size_t nCount) throw()
{
    ff32Only(nCount = (nCount + 1) & ~size_t(1));//increment 2 nearest even
    size_t nSize = sizeof(SeqNodeDAndOr) + sizeof(void*) * (nCount - 1);
    ffAssume(0 == (nSize & 7));
    return nSize;
}                                        

ffForceInline
size_t SeqNodeLAnd::SizeOf(size_t nCount) throw()
{
    ff32Only(nCount = (nCount & ~size_t(1)) + 1);//increment 2 nearest odd     
    size_t nSize = sizeof(SeqNodeLAnd) + sizeof(void*) * (nCount - 1);
    ffAssume(0 == (nSize & 7));
    return nSize;
}

ffForceInline
size_t SeqNodeLOr::SizeOf(size_t nCount) throw()
{
    ff32Only(nCount = (nCount + 1) & ~size_t(1));//increment 2 nearest even
    size_t nSize = sizeof(SeqNodeLOr) + sizeof(void*) * (nCount - 1);
    ffAssume(0 == (nSize & 7));
    return nSize;
}


}//namespace Search
}//namespace FastFish

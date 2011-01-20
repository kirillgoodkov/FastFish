namespace FastFish{
namespace Build{

template<typename Node>
void GatherStatistic::operator()(char ch, const Node* pNode) throw()
{
    m_str.push_back(ch);
    m_str.append(pNode->Suffix(), pNode->Suffix() + pNode->SuffixLen());
    
    if (m_nLimit <= pNode->Postings().Count())
    {
        ffLog(lsInfoTopPrefix, "%9u\t%s\n", pNode->Postings().Count(), m_str.c_str());
    }
    
    pNode->Childs().Enum(*this);    
    m_str.resize(m_str.size() - pNode->SuffixLen() - 1);
}

}//namespace Build
}//namespace FastFish

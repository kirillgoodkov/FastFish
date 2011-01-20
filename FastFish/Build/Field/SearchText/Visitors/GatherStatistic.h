#pragma once
#include "Common/Common.h"

namespace FastFish{
namespace Build{

class GatherStatistic
{
    GatherStatistic(const GatherStatistic&);
    GatherStatistic& operator = (const GatherStatistic&);
public:
    GatherStatistic(docid_t nLimit) throw(): m_nLimit(nLimit){}        

    template<typename Node>
    void operator()(char ch, const Node* pNode) throw();

private:    
    docid_t             m_nLimit;
    std::string         m_str;          
};


}//namespace Build
}//namespace FastFish

#include "Build/Field/SearchText/Visitors/GatherStatistic.inl"

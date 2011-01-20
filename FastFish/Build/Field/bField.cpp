#include "Build/Field/bField.h"
#include "Build/Field/DataFix/bFieldDataFix.h"
#include "Build/Field/DataFix/bFieldDataFixOpt.h"
#include "Build/Field/DataVar/bFieldDataVar.h"
#include "Build/Field/SearchText/bFieldSearchText.h"
#include "Build/Field/SearchText/Reduce/Splitter.h"
#include "Build/Field/SearchText/Reduce/ReducerSuffixTree.h"
#include "Build/Field/SearchText/Reduce/ReducerNop.h"
#include "Common/Templates.h"
#include "Common/Consts.h"

using namespace FastFish::Build::Reduce;

namespace FastFish{
namespace Build{

Field* CreateFieldDataFix(const TplIndex& tplIndex, const TplFieldDataFix& tpl, const Documents* pDocs) throw()
{
    if (tpl.bOptional)
        return new FieldDataFixOpt(tplIndex, tpl, pDocs);
    else
        return new FieldDataFix(tplIndex, tpl, pDocs);
}

Field* CreateFieldDataVar(const TplIndex& tplIndex, const TplFieldDataVar& tpl, const Documents* pDocs) throw()
{
    if (tpl.bZeroTerminated)
    {
        if (tpl.bStoreZero)
            return new FieldDataVar<true, true>(tplIndex, tpl, pDocs);
        else
            return new FieldDataVar<true, false>(tplIndex, tpl, pDocs);            
    }        
    else
    {
        return new FieldDataVar<false, false>(tplIndex, tpl, pDocs);        
    }        
}

Field* CreateFieldSearchText(const TplIndex& tplIndex, const TplFieldSearchText& tpl, const Documents* pDocs, TextAllocator& a) throw()
{
    if (tpl.bTextWithControls)
    {
        if (tpl.bTextCaseSensitive)
            switch (tpl.titType)
            {
                case titExact:  return new Splitter<ReducerNop<FieldSearchText<true, true, titExact >, true, true>, true, true, titExact >(tplIndex, tpl, pDocs, a);
                case titPrefix: return new Splitter<ReducerSuffixTree<FieldSearchText<true, true, titPrefix>, true, true, titPrefix>, true, true, titPrefix>(tplIndex, tpl, pDocs, a);
                case titInfix:  return new Splitter<ReducerSuffixTree<FieldSearchText<true, true, titInfix >, true, true, titInfix >, true, true, titInfix >(tplIndex, tpl, pDocs, a);
                default: ffAssume(false);                                                                  
            }            
        else
            switch (tpl.titType)
            {
                case titExact:  return new Splitter<ReducerNop<FieldSearchText<true, false, titExact >, true, false>, true, false, titExact >(tplIndex, tpl, pDocs, a);
                case titPrefix: return new Splitter<ReducerSuffixTree<FieldSearchText<true, false, titPrefix>, true, false, titPrefix>, true, false, titPrefix>(tplIndex, tpl, pDocs, a);
                case titInfix:  return new Splitter<ReducerSuffixTree<FieldSearchText<true, false, titInfix >, true, false, titInfix >, true, false, titInfix >(tplIndex, tpl, pDocs, a);
                default: ffAssume(false);                               
            }            
    }
    else
    {
        if (tpl.bTextCaseSensitive)
            switch (tpl.titType)
            {
                case titExact:  return new Splitter<ReducerNop<FieldSearchText<false, true, titExact >, false, true>, false, true, titExact >(tplIndex, tpl, pDocs, a);
                case titPrefix: return new Splitter<ReducerSuffixTree<FieldSearchText<false, true, titPrefix>, false, true, titPrefix>, false, true, titPrefix>(tplIndex, tpl, pDocs, a);
                case titInfix:  return new Splitter<ReducerSuffixTree<FieldSearchText<false, true, titInfix >, false, true, titInfix >, false, true, titInfix >(tplIndex, tpl, pDocs, a);
                default: ffAssume(false);                                                                  
            }            
        else
            switch (tpl.titType)
            {
                case titExact:  return new Splitter<ReducerNop<FieldSearchText<false, false, titExact >, false, false>, false, false, titExact >(tplIndex, tpl, pDocs, a);
                case titPrefix: return new Splitter<ReducerSuffixTree<FieldSearchText<false, false, titPrefix>, false, false, titPrefix>, false, false, titPrefix>(tplIndex, tpl, pDocs, a);
                case titInfix:  return new Splitter<ReducerSuffixTree<FieldSearchText<false, false, titInfix >, false, false, titInfix >, false, false, titInfix >(tplIndex, tpl, pDocs, a);
                default: ffAssume(false);                                                                                  
            }            
    }        
    ffReturnNop;
}

TextAllocator::TextAllocator() throw():
    aMap(sizeof(Node<false, false, titExact>)),
    aSet(PC::NodeAllocSetMul * PC::SetBlockSize, PC::SetBlockSize, PC::SetBlockSize),
    aStr(PC::NodeAllocStrSize),
    aRMap(PC::ReducerMapAllocSize, 1, sizeof(void*)),
    aRStr(PC::ReducerStrAllocSize),
    aSkipMap(PC::ReducerMapSkipAllocSize, 1, sizeof(void*)),
    aSkipStr(PC::ReducerSetSkipAllocSize)
{
}

}//namespace Build
}//namespace FastFish

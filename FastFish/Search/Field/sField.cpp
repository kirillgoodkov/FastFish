#include "Search/Field/sField.h"
#include "Search/Field/SearchText/sFieldSearchText.h"
#include "Search/Field/DataFix/sFieldDataFix.h"
#include "Search/Field/DataFix/sFieldDataFixOpt.h"
#include "Search/Field/DataVar/sFieldDataVar.h"
#include "Search/Documents/sDocuments.h"

namespace FastFish{
namespace Search{

Field* CreateField(const TplFieldDataFix& tpl, Documents* /*pDocs*/) throw()
{
    if (tpl.bOptional)
        return new FieldDataFixOpt(tpl);
    else
        return new FieldDataFix(tpl);
}
Field* CreateField(const TplFieldDataVar& tpl, Documents* /*pDocs*/) throw()
{
    return new FieldDataVar(tpl);
}

Field* CreateField(const TplIndex& /*tplIndex*/, const TplFieldSearchText& tpl, const Documents* pDocs) throw()
{
    if (titExact == tpl.titType)
    {
        if (tpl.bTextCaseSensitive)
            return new FieldSearchText<true, true, titExact>(tpl, pDocs);
        else            
            return new FieldSearchText<false, true, titExact>(tpl, pDocs);        
    }
    
    if (tpl.bTextCaseSensitive)
    {
        if (1 < tpl.nWordLenMin)
        {
            if (titPrefix == tpl.titType)
                return new FieldSearchText<true, true, titPrefix>(tpl, pDocs);
            else
                return new FieldSearchText<true, true, titInfix >(tpl, pDocs);                        
        }
        else
        {
            if (titPrefix == tpl.titType)
                return new FieldSearchText<true, false, titPrefix>(tpl, pDocs);
            else
                return new FieldSearchText<true, false, titInfix >(tpl, pDocs);                        
        }
    }
    else
    {
        if (1 < tpl.nWordLenMin)
        {
            if (titPrefix == tpl.titType)
                return new FieldSearchText<false, true, titPrefix>(tpl, pDocs);
            else
                return new FieldSearchText<false, true, titInfix >(tpl, pDocs);                        
        }
        else
        {
            if (titPrefix == tpl.titType)
                return new FieldSearchText<false, false, titPrefix>(tpl, pDocs);
            else
                return new FieldSearchText<false, false, titInfix >(tpl, pDocs);                        
        }
    }
}

}//namespace Search
}//namespace FastFish

#include "EventFramework.h"

template<>
void LL<Event>::doItems()
{
  for(LL<Event>* pLL = begin(); pLL != end(); )
    if(!pLL->pItem->callback())
      pLL = pLL->erase();         // remove from list
}


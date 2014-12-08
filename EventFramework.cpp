#include "EventFramework.h"
namespace efl {

template<>
void LL<Event>::doItems()
{
  for(LL<Event>* pLL = begin(); pLL != end(); )
    if(!pLL->pItem->callback())
      pLL = pLL->erase();         // remove from list
}

template<>
void LL<Timer>::doItems()
{
  static ulong prevMillis = 0;    // start when execution starts
  ulong   nowMillis = millis();   // time now
  ulong   deltaMillis = nowMillis - prevMillis;

  if(!deltaMillis)
    return;

  // iterate through timers to see which ones have down counted to or beyond zero
  for(LL<Timer>* pLL = begin(); pLL != end(); )
  {
    ulong late=deltaMillis - pLL->pItem->getCounter();
    if( pLL->pItem->getCounter() <= deltaMillis )
    {
      if( pLL->pItem->callback(late) && pLL->pItem->getPeriod() > 0 ) // need both period and 'true' response to keep active
      {
        // policy decision here. Do we set the counter to 0 or less if
        // we're late by the period or more? No, I guess...
        if( late >= pLL->pItem->getPeriod())
          pLL->pItem->setCounter(1);
        else
          pLL->pItem->setCounter(pLL->pItem->getPeriod()-late);
        pLL = pLL->next();
      }
      else
      {
        pLL = pLL->erase();
      }
    }
    else
    {
      pLL->pItem->setCounter(pLL->pItem->getCounter()-deltaMillis);
      pLL = pLL->next();
    }
  }
  prevMillis = nowMillis;
}

template<>
void LL<Digital>::doItems() {
     static ulong prevMillis = 0;    // start when execution starts
     ulong   nowMillis = millis();   // time now
     uint   deltaMillis = nowMillis - prevMillis;
 
     if(!deltaMillis)
       return;
 
 // iterate through timers to see which ones have downcounted to or beyond zero
     for(LL<Digital>* pLL = begin(); pLL != end(); pLL = pLL->next()) {
       // scan all digital inputs
         switch(pLL->pItem->getState()) {
             case Digital::INACTIVE:
                if( pLL->pItem->getSense()) {
                    //Printf( "pItem->id(%d) going active.\n", pLL->pItem->getID());
                    if( pLL->pItem->getDebounce() > 0 ) {
                        pLL->pItem->setState(Digital::GOING_ACTIVE);
                        pLL->pItem->setDebounceCounter();
                    }
                    else {
                        pLL->pItem->setState(Digital::ACTIVE);
                    }
                }
                break;

             case Digital::GOING_INACTIVE:
             case Digital::GOING_ACTIVE:
                if( pLL->pItem->decrementDebounce(deltaMillis) <= 0) {
                    //Printf("Debounce complete id(%d)\n", pLL->pItem->getID());
                    if( pLL->pItem->getSense() )
                        pLL->pItem->setState(Digital::ACTIVE);
                    else
                        pLL->pItem->setState(Digital::INACTIVE);
                 }
                 break;

             case Digital::ACTIVE:
                if( !pLL->pItem->getSense()) {
                    //Printf( "pItem->id(%d) going inactive.\n", pLL->pItem->getID());
                    if( pLL->pItem->getDebounce() > 0 ) {
                        pLL->pItem->setState(Digital::GOING_INACTIVE);
                        pLL->pItem->setDebounceCounter();
                    }
                    else {
                        pLL->pItem->setState(Digital::INACTIVE);
                    }
                }
                 break;

             default:
                 pLL->pItem->setState(Digital::INACTIVE); // what else to do here?
                 break;
          }
      }
    prevMillis = nowMillis;
}


} // namespace efl

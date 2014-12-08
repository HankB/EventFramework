namespace efl { // event framework library

/**
 * LL is a template class which adds the linked list to objects of type Item
 * which are the various Event types. It use a singly linked list to minimize
 * RAM footprint. Lists are expected to be relatively short so traversing the
 * list from the start to find the previous element it should mot take too
 * long. It uses a sentinel to point tothe first real item in the list and the
 * last item in the list points to the sentinel.
 */

typedef unsigned long int ulong;
typedef unsigned int uint;
typedef unsigned char uchar;

template<class Item> class LL {
private:
  LL* 	    pNext;          // point to next item in list
  static LL<Item>& sentinel() {
    static LL<Item> rc=LL<Item>((Item*)0);
    return rc;
  };
  Item*    pItem;                 // event descriptor
  LL():
  pItem((Item*)0),pNext(this) {
  }; // can't construct w/out an Item except for the special case in sentinel()
public:
  // return status of list manipulation operations
  enum rs {
    OK=0,                   // return status - operation succeeded
    NAK=1,                  // general bad status
    BAD_DUP=2               // duplicate e.g already in a list.
  };

  LL(Item*pI):
  pNext(this),pItem(pI) {
  };   // construct from an Item
  rs add();                               // add an Item to the tail of the queue
  rs push();                              // push an Item on to he front of the queue
  LL* next() {
    return pNext;
  };              // return pointer to next iutem in list
  LL* previous();                         // pointer to previous item in list
  static LL* begin() {
    return sentinel().pNext;
  };   // pointer to first item in list (past the sentinel)
  static LL* end() {
    return &sentinel();
  };  // pointer to end of list (e.g. the sentinel)
  LL* erase();                            // remove this item from the list
  static int& size() {
    static int elementCount=0;
    return elementCount;
  };
  static void doItems();      // each specialization expected to provide their own 'doItems()'

#if !defined AVR
  static void walk();
#endif
};

template<class Item>
typename LL<Item>::rs LL<Item>::add() {
  if( this == end())            // this would be bad!
    return BAD_DUP;
  if (pNext == this) {                // should be point to itself right now
    LL<Item>* pLL = LL<Item>::sentinel().pNext; // point to head of list

      while (pLL->pNext != &LL<Item>::sentinel()) // Does this one point toward the sentinel (i.e. end of list)
      pLL = pLL->pNext;

    pLL->pNext = this;              // make end of list point to the one to add
    pNext = &LL<Item>::sentinel();     // and now mark it as the new end of the list
    size()++;
    return LL<Item>::OK;
  } 
  else {
    return LL<Item>::BAD_DUP;
  }
}

template<class Item>
typename LL<Item>::rs LL<Item>::push()
{
  if( this != pNext ) // already in the list
    return BAD_DUP;
  pNext = sentinel().pNext;
  sentinel().pNext = this;
  size()++;
  return OK;
}


template<class Item>
LL<Item>* LL<Item>::previous()
{
  LL<Item>* pLL = begin();
  while(pLL != end())
    if(pLL->next() == this)
      return pLL;
    else
      pLL = pLL->next();
  return pLL;
}

#if !defined AVR

template<class Item>
void LL<Item>::walk()
{
  coln("");
  co("walk(");
  co(size());
  co(") ");
  LL<Item>*  pLL = LL<Item>::sentinel().pNext;
  co( (void*)&sentinel() );
  co( " -> ");
  while( pLL != &LL<Item>::sentinel() )
  {
    co( pLL);
    co( " -> ");
    pLL = pLL->pNext;
    //sleep(sleepTime);
  }
  coln( pLL );
}

#endif

template<class Item>
LL<Item>* LL<Item>::erase()
{
  if( this == pNext )                           // marked as not in list?
    return begin();                           // does anything else make sense here? This should be safe for the caller.
  LL<Item>*  pLL = &LL<Item>::sentinel();       // start at the head end of the list
  while( pLL->pNext != &LL<Item>::sentinel() )  // and iterate until we've gone through the list (should never happen!
  {
    if( this == pLL->pNext )
    {
      pLL->pNext = pNext;
      pNext = this;
      size()--;
      return pLL->pNext;
    }
    pLL = pLL->pNext;
  }
  assert(0);
  return pLL->pNext;      // should never get here
}

/*
 * Generic Event - one can just chain a bunch of these together and
 * execute them. Not very interesting but the simplest case.
 */
class Event { // callback makes this a one shot event (won't execute until it is added back to the list.)
public:

  Event() {
  };
  virtual bool callback() {
    if (verbose) coln( "Event:");
    return false;
  };
};


/*
 * Timer class gets a little more interesting. The default behavior
 * is a one shot but given a non-zero period will be periodic.
 *
 */
class Timer {
private:
  ulong   counter;
  ulong   period;
public:
  Timer(ulong c=1, ulong p=0):
  counter(c),period(p) {
  }; // default to fire once after 1 ms
  virtual bool callback(ulong late) {
    if (verbose) {
      coln( "Timer:");
    }
    return false;
  };
  ulong getCounter() {
    return counter;
  }
  void setCounter(ulong c) {
    counter=c;
  }
  ulong getPeriod() {
    return period;
  }
  void setPeriod(ulong p) {
    period=p;
  }

};

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
#define DIGITAL
#if defined DIGITAL

/**
 * Provide a framework to respond to changes in state of digital inputs including 
 * debouncing the input for a specified time (in milliseconds.)
 */

class Digital
{
public:
  typedef enum      /// interpreted state of the input
  {
    INACTIVE =      (1<<0),	//1
    GOING_ACTIVE =  (1<<1),	//2
    ACTIVE =        (1<<2),	//4
    GOING_INACTIVE =(1<<3),	//8
  } States;

  typedef enum/// input polarity
  {
    ACT_LO,
    ACT_HI,
  } Polarity;

  typedef enum      /// valid digital input bits for an Arduino Uno
  {                 // an enumerated value is chosen to
      BIT_1 = 1,    // provide some guarantee that a valid
      BIT_2,        // bit is specified.
      BIT_3,
      BIT_4,
      BIT_5,
      BIT_6,
      BIT_7,
      BIT_8,
      BIT_9,
      BIT_10,
      BIT_11,
      BIT_12,
      BIT_13,
      AN_0,    // pseudo digital inputs, I suppose.
      AN_1,
      AN_2,
      AN_3,
      AN_4,
      AN_5,
  } DigitalBit;
 

private:
  int           id;
  uint          debounce;
  int           debounceCounter;
  States        state;
  Polarity      polarity;
  DigitalBit    pin;
  uchar         interestMask;

  

public:
  Digital( int id, DigitalBit b,int d = 1, Polarity p = ACT_HI, uchar interest = (INACTIVE|ACTIVE)):
  id(id), debounce(d), state(INACTIVE), polarity(p), pin(b), interestMask(interest)
  {
     pinMode(pin, INPUT);      // should this be done in setup?
  };				// defaults: 1 ms debounce and active high polarity
                    // and interested transitions to inactive or active only

  States getState() { return state;};

    /*
     * I hate to pass in the flag to indicate that this is a 'significant' change worthy of a callback
     * but the logic is so much easier where the states are managed.
     */
  void setState(States s) {
      States oldState = state;
      state=s;
      if( interestMask & state)
          callback(0, state, oldState);
  };
  bool getSense() { return (polarity==ACT_HI)?digitalRead(pin):!digitalRead(pin); };
  int getID() { return id; };
  uint setDebounceCounter() { return debounceCounter = debounce; };
  uint getDebounce() { return debounce; };
  int decrementDebounce(uint delta) { return debounceCounter -= delta; };
  virtual bool callback (ulong late, States newState, States oldState) {   /// callback on state changes

    if (verbose)
	  coln (X("Digital:"));
    return false;
  };
};

template<>
void LL<Digital>::doItems() {
     static ulong prevMillis = 0;    // start when execution starts
     ulong   nowMillis = millis();   // time now
     uint   deltaMillis = nowMillis - prevMillis;
 
     if(!deltaMillis)
       return;
 
//     if(verbose) walk();
     
 // iterate through timers to see which ones have downcounted to or beyond zero
     for(LL<Digital>* pLL = begin(); pLL != end(); pLL = pLL->next()) {
       // scan all digital inputs
         switch(pLL->pItem->getState()) {
             case Digital::INACTIVE:
                if( pLL->pItem->getSense()) {
                    Printf( "pItem->id(%d) going active.\n", pLL->pItem->getID());
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
                    Printf("Debounce complete id(%d)\n", pLL->pItem->getID());
                    if( pLL->pItem->getSense() )
                        pLL->pItem->setState(Digital::ACTIVE);
                    else
                        pLL->pItem->setState(Digital::INACTIVE);
                 }
                 break;

             case Digital::ACTIVE:
                if( !pLL->pItem->getSense()) {
                    Printf( "pItem->id(%d) going inactive.\n", pLL->pItem->getID());
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

#endif //defined DIGITAL

} // namespace efl



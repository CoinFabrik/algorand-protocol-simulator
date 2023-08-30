/*
 *
 *  Created on: 28 ago. 2023
 *      Author: argic
 */

#include "CustomEventHeap.h"

#define TEST 1


Register_Class(CustomEventHeap);


CustomEventHeap::CustomEventHeap(const char* name):cEventHeap(nullptr,128)
{
//    setUseCb(false);
}


CustomEventHeap::~CustomEventHeap()
{
    clear();
}


void CustomEventHeap::insert(cEvent* event)
{
#if TEST
    event->setHeapIndex(0);
    take(event);
    Events.insert(event);
#else
    cEventHeap::insert(event);
#endif
}


cEvent* CustomEventHeap::peekFirst() const
{
#if TEST
    return Events.peekFirst();
#else
    cEvent* ret = cEventHeap::peekFirst();
    return ret;
#endif
}


cEvent* CustomEventHeap::removeFirst()
{
#if TEST
    cEvent* ev = Events.removeFirst();
    ev->setHeapIndex(-1);
    drop(ev);
    return ev;
#else
    cEvent* ret = cEventHeap::removeFirst();
    return ret;
#endif
}


void CustomEventHeap::putBackFirst(cEvent *event)
{
#if TEST
    event->setHeapIndex(0);
    take(event);
    Events.insert(event);
#else
    cEventHeap::putBackFirst(event);
#endif
}


cEvent* CustomEventHeap::remove(cEvent *event)
{
#if TEST
    event->setHeapIndex(-1);
    Events.remove(event);
    drop(event);
    return event;
#else
    return cEventHeap::remove(event);
#endif
}


bool CustomEventHeap::isEmpty() const
{
#if TEST
    return Events.length() == 0;
#else
    return cEventHeap::isEmpty();
#endif
}


void CustomEventHeap::clear()
{
#if TEST
    while(Events.length())
        dropAndDelete(Events.removeFirst());
#else
    cEventHeap::clear();
#endif
}


int CustomEventHeap::getLength() const
{
#if TEST
    return Events.length();
#else
    return cEventHeap::getLength();
#endif
}


cEvent* CustomEventHeap::get(int k)
{
#if TEST
    return nullptr;
#else
        return cEventHeap::get(k);
#endif
}


void CustomEventHeap::sort()
{
#if TEST
    return;
#else
    cEventHeap::sort();
#endif
}

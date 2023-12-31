/*
 * CustomEventHeap.h
 *
 *  Created on: 28 ago. 2023
 *      Author: argic
 */

#ifndef CUSTOMEVENTHEAP_H_
#define CUSTOMEVENTHEAP_H_

#include <omnetpp.h>
#include <vector>
#include <deque>
#include <set>
#include <algorithm>

using namespace omnetpp;


struct evCmp
{
    bool operator() (const cEvent* lhs, const cEvent* rhs) const
    {
        return lhs->getArrivalTime() < rhs->getArrivalTime();
    }
};


struct DataStructure
{
    std::vector<std::multiset<cEvent*, evCmp>> data;
    int min_idx = -1;
    int size = 0;

    DataStructure()
    {
        data.resize(10000);
//        for (int i = 0; i < 1000; i++)
//            data[i].reserve(4000);
    }

    cEvent* peekFirst() const
    {
        if (min_idx >= 0)
            return (*data[min_idx].begin());
        else return nullptr;
    }

    void insert(cEvent* ev)
    {
        int evIdx = ev->getArrivalTime().inUnit(SIMTIME_S);
        data[evIdx].insert(ev);
        if (peekFirst() == nullptr || peekFirst()->getArrivalTime() > ev->getArrivalTime())
                min_idx = evIdx;
        size++;
    }

    cEvent* removeFirst()
    {
        if (min_idx == -1) return nullptr;
        cEvent* ev = (*data[min_idx].begin());
        data[min_idx].erase(data[min_idx].begin());
        size--;
        if (size == 0) min_idx = -1;
        else
        {
            for (; min_idx < data.size(); min_idx++)
                if (data[min_idx].size())
                    break;
        }
        return ev;
    }

    void remove(cEvent* ev)
    {
        if (min_idx == -1) return;
        int evIdx = ev->getArrivalTime().inUnit(SIMTIME_S);

        for (auto it = data[evIdx].begin(); it != data[evIdx].end(); it++)
            if (*it == ev)
            {
                data[evIdx].erase(it);
                break;
            }

        size--;
        if (size == 0) min_idx = -1;
        else if (min_idx == evIdx && data[evIdx].size() == 0)
        {
            for (; min_idx < data.size(); min_idx++)
                if (data[min_idx].size())
                    break;
        }
    }

    int length() const{return size;}
};


class CustomEventHeap : public cEventHeap{
public:
    CustomEventHeap(const char* name = nullptr);
    virtual ~CustomEventHeap();

    /**
     * Insert an event into the FES.
     */
    virtual void insert(cEvent *event) override;

    /**
     * Peek the first event in the FES (the one with the smallest timestamp.)
     * If the FES is empty, it returns nullptr.
     */
    virtual cEvent *peekFirst() const override;

    /**
     * Removes and return the first event in the FES (the one with the
     * smallest timestamp.) If the FES is empty, it returns nullptr.
     */
    virtual cEvent *removeFirst() override;

    /**
     * Undo for removeFirst(): it puts back an event to the front of the FES.
     */
    virtual void putBackFirst(cEvent *event) override;

    /**
     * Removes and returns the given event in the FES. If the event is
     * not in the FES, returns nullptr.
     */
    virtual cEvent *remove(cEvent *event) override;

    /**
     * Returns true if the FES is empty.
     */
    virtual bool isEmpty() const override;

    /**
     * Deletes all events in the FES.
     */
    virtual void clear() override;


    /**
     * Returns the number of events in the FES.
     */
    virtual int getLength() const override;

    /**
     * Returns the kth event in the FES if 0 <= k < getLength(), and nullptr
     * otherwise. Note that iteration does not necessarily return events
     * in increasing timestamp (getArrivalTime()) order unless you called
     * sort() before.
     */
    virtual cEvent *get(int k) override;

    /**
     * Sorts the contents of the FES. This is only necessary if one wants
     * to iterate through in the FES in strict timestamp order.
     */
    virtual void sort() override;


    DataStructure Events;
};

#endif /* CUSTOMEVENTHEAP_H_ */

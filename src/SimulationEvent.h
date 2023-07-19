#ifndef SIMULATIONEVENT_H_
#define SIMULATIONEVENT_H_

#include <omnetpp.h>

using namespace omnetpp;


class SimulationEvent {
public:
    SimulationEvent(){}
    virtual ~SimulationEvent(){}

    virtual void ExecuteEvent(){}
};


class TimeTriggeredSimulationEvent : public SimulationEvent
{
    //incluye un trigger time
    SimTime TriggerTime;
    cMessage* SchedulingMessage;

public:
    virtual ~TimeTriggeredSimulationEvent(){}
    void Schedule();
};


class RoundTriggeredSimulationEvent : public SimulationEvent
{
    //incluye un target round
    uint64_t TriggerRound;
};


class CustomTriggeredSimulationEvent : public SimulationEvent
{
    virtual bool CheckTriggerCondition(){return false; }


};


#endif /* SIMULATIONEVENT_H_ */

#ifndef SIMULATIONEVENT_H_
#define SIMULATIONEVENT_H_

#include <omnetpp.h>

#ifndef GLOBALSIMULATIONMANAGER_H_
    #include "GlobalSimulationManager.h"
#endif

#ifndef PARTICIPATIONNODE_H_
    #include "ParticipationNode.h"
#endif

#ifndef DATATYPEDEFINITIONS_H_
    #include "DataTypeDefinitions.h"
#endif

using namespace omnetpp;


class SimulationEvent {
public:
    SimulationEvent(){}
    virtual ~SimulationEvent(){}

    virtual void ExecuteEvent(){}
};


class TimeTriggeredSimulationEvent : public SimulationEvent
{
public:
    //incluye un trigger time
    SimTime TriggerTime;
    cMessage* SchedulingMessage;

public:
    virtual ~TimeTriggeredSimulationEvent(){}
    void Schedule();
};




class PartitionSimulationEvent : public TimeTriggeredSimulationEvent
{
public:
    static class NetworkDefinition cachedNetworkState;
    void ExecuteEvent();
};


class ReconnectionSimulationEvent : public TimeTriggeredSimulationEvent
{
public:
    void ExecuteEvent();
};




class CustomTriggeredSimulationEvent : public SimulationEvent
{
public:
    virtual bool CheckTriggerCondition(){return false; }


};

#endif /* SIMULATIONEVENT_H_ */

#include "SimulationEvent.h"


void TimeTriggeredSimulationEvent::Schedule()
{
//    GlobalSimulationManager::SimManager->ScheduleAt();
}




NetworkDefinition PartitionSimulationEvent::cachedNetworkState;


void PartitionSimulationEvent::ExecuteEvent()
{
    auto* sim = GlobalSimulationManager::SimManager;

    cachedNetworkState.ParticipationNodes.clear();
    cachedNetworkState.RelayNodes.clear();
    for (auto partPtr : sim->Network.ParticipationNodes)
        cachedNetworkState.ParticipationNodes.push_back(partPtr);
    for (auto relID : sim->Network.RelayNodes)
        cachedNetworkState.RelayNodes.push_back(relID);


    int partMod = 20;

    for (int i = 0; i < sim->Network.RelayNodes.size(); i++)
    {
        auto mod = [i, partMod](int t) { return (t % partMod) != (i % partMod); };

        auto p1 = std::remove_if(sim->Network.RelayNodes[i].RelayConnections.begin(), sim->Network.RelayNodes[i].RelayConnections.end(), mod);
        auto p2 = std::remove_if(sim->Network.RelayNodes[i].ParticipationConnections.begin(), sim->Network.RelayNodes[i].ParticipationConnections.end(), mod);

        sim->Network.RelayNodes[i].RelayConnections.erase(p1, sim->Network.RelayNodes[i].RelayConnections.end());
        sim->Network.RelayNodes[i].ParticipationConnections.erase(p2, sim->Network.RelayNodes[i].ParticipationConnections.end());
    }

//    for (int i = 0; i < sim->Network.ParticipationNodes.size(); i++)
//    {
//        auto mod = [i, partMod](int t) { return (t % partMod) != (i % partMod); };
//
//        auto p1 = std::remove_if(sim->Network.ParticipationNodes[i]->RelayConnections.begin(), sim->Network.ParticipationNodes[i]->RelayConnections.end(), mod);
//        sim->Network.ParticipationNodes[i]->RelayConnections.erase(p1, sim->Network.ParticipationNodes[i]->RelayConnections.end());
//    }

    EV << "PARTITIONED " << endl;
}


void ReconnectionSimulationEvent::ExecuteEvent()
{
    auto* sim = GlobalSimulationManager::SimManager;
    sim->Network = PartitionSimulationEvent::cachedNetworkState;

    EV << "RECONNECTED " << endl;
}

//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "GlobalSimulationManager.h"
#include "ParticipationNode.h"


Define_Module(ParticipationNode);


ParticipationNode::ParticipationNode() : round(1), period(0), step(0), lastConcludingStep(0) //, pinnedVote(0)
{
    FastResyncEvent = new cMessage(nullptr, 253);
    TimeoutEvent = new cMessage(nullptr, 0);

    FreshestBundle = nullptr;
    SigmaBundle = nullptr;
    MuValue = EMPTY_PROPOSAL_VALUE;
}


ParticipationNode::~ParticipationNode()
{
    cancelAndDelete(FastResyncEvent);
    cancelAndDelete(TimeoutEvent);
}


void ParticipationNode::initialize()
{
    //initialize constant for sortition
    two_to_the_hashlen = boost::multiprecision::pow(BigFloat(2.0), 64*8);
    generator.seed(getIndex());

    //Timeout starter self-event
    scheduleAfter(0, TimeoutEvent);

    InitOnlineAccounts();
}


void ParticipationNode::InitOnlineAccounts()
{
    for (int i = 0; i < uint64_t(TOTAL_ACCOUNTS); i++)
    {
        auto NewAccount = Account(rand(), uint64_t(START_MONEY));
        onlineAccounts.push_back(NewAccount);

        //initialize global balance tracker
        BalanceMap[NewAccount.I] =  BalanceRecord(NewAccount.Money, true);

        GlobalSimulationManager::SimManager->BalanceMap[NewAccount.I] =  BalanceRecord(NewAccount.Money, true);
    }
}


void ParticipationNode::UpdateBalances()
{
#if GLOBAL_BALANCE_TRACKER
    //if tracking balance globally, we only let one node (that we are certain is honest, and should not be tampered with) update the global balance
    //in the future maybe handle this with a global manager class, with access to all nodes that could get the "real" ledger by majority
    if (this->getIndex() != 0)
        return;
#endif

    if (!Ledger.Entries.size()) return;

    uint64_t LookbackParam = 0;
    //get transactions from last addition to the ledger
    auto& LastBlock = *(Ledger.Entries.end()-1);
    auto& LookbackBlock = *(Ledger.Entries.end()-(Ledger.Entries.size()>=LookbackParam?LookbackParam:0));

    //update all balances according to last block
    for (Transaction& txn : LastBlock.Txns)
    {
        //por ahora solo pay
        BalanceMap[txn.Sender].RawBalance -= txn.Fee + txn.Amount;
        BalanceMap[txn.Receiver].RawBalance += txn.Amount;
//        Balance[FEE_SINK_ADDRESS].RawBalance += txn.Fee;
    }

    //updatear all cached balances according to lookback block
    for (Transaction& txn : LookbackBlock.Txns)
    {
        //por ahora solo pay
        BalanceMap[txn.Sender].OldBalance -= txn.Fee + txn.Amount;
        BalanceMap[txn.Receiver].OldBalance += txn.Amount;
    }
}


void ParticipationNode::SimulateTransactions()
{
    //for now, generate 10 random txns
    for (int i = 0; i < 10; i++)
    {
        Transaction txn = GenerateRandomTransaction();
        Broadcast(txn);
    }
}


Transaction ParticipationNode::GenerateRandomTransaction()
{
    Transaction txn;
    //pick random txnID, increase global counter
    //pick random sender
    //pick random receiver
    //pick random amount (amount <= sender balance)
    return txn;
}




void ParticipationNode::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) //timeout event
    {
        if (msg->getKind() == 0)
        {
            //TESTING--
            //txn sim stuff. Testing
            SimulateTransactions();

            //block assembly stuff. Testing
            auto e = BlockAssembly();
            //TESTING--


            startTime = simTime();
            step = 0;

            //schedule a soft vote
            TimeoutEvent->setKind(1);
            scheduleAfter(FilterTimeout(period), TimeoutEvent);
            //schedule a fast recovery attempt
            scheduleAfter(LambdaF + 0, FastResyncEvent);

            BlockProposal(e);

            // //pin value? check on specs
            // pinnedPayload.e = LocalBlock;

            lastConcludingStep = 0;
            step = 1;
            //aca ya propuse, y me quedo esperando el soft
        }
        else if (msg->getKind() == 1)
        {
            step = 1;

            TimeoutEvent->setKind(3);
            scheduleAt(startTime + fmax(4.f * Lambda, UppercaseLambda), TimeoutEvent);

            SoftVote();
            EV << "SOFT" << endl;

            step = 2;
            lastConcludingStep = 1;

            //certification vote happens from now on, but is driven by messages (not timeout)
            //node has to observe a cert bundle BEFORE the next timeout
        }
        else if (msg->getKind() < 253)  // >= 3 por definicion
        {
            EV << "NEXT: " << int(msg->getKind()) << endl;

            lastConcludingStep = step;
            step = msg->getKind();

            //252 is the last next-vote. After this, its periodic cycles of late, recovery, redo
            if (msg->getKind() <= 251)
            {
                TimeoutEvent->setKind(step + 1);
                scheduleAt(startTime + fmax(4.f * Lambda, UppercaseLambda) + pow(2.f, step) * Lambda + 0, TimeoutEvent);
            }

            NextVote();
        }
        else //msg->getKind() >= 253
        {
            lastConcludingStep = step;
            step = 255;
            scheduleAfter(LambdaF + 0, FastResyncEvent);

            FastRecovery();
        }
    }
}


void ParticipationNode::Broadcast(Bundle& b)
{
    for (Vote& vt : b.votes)
            Broadcast(vt);
}


void ParticipationNode::Broadcast(ProposalPayload& pp)
{
    //broadcast a given proposal
    for (int i = 0; i < TOTAL_NODES; i++)
    {
        std::string nodePath = "AlgorandNetwork.PartNode[" + std::to_string(i) + "]";
        ParticipationNode* bro = (ParticipationNode*)(getModuleByPath(nodePath.c_str()));
        bro->HandleProposal(pp);
    }
}


void ParticipationNode::Broadcast(Vote& v)
{
    //broadcast a given proposal
    for (int i = 0; i < TOTAL_NODES; i++)
    {
        std::string nodePath = "AlgorandNetwork.PartNode[" + std::to_string(i) + "]";
        ParticipationNode* bro = (ParticipationNode*)(getModuleByPath(nodePath.c_str()));
        bro->HandleVote(v);
    }
}


void ParticipationNode::Broadcast(Transaction& txn)
{
    GlobalSimulationManager::SimManager->Network.PropagateMessageThroughNetwork(RelayConnections, getIndex(), (void*)(&txn), TXN);

    //broadcast a given proposal
    for (int i = 0; i < TOTAL_NODES; i++)
    {
        std::string nodePath = "AlgorandNetwork.PartNode[" + std::to_string(i) + "]";
        ParticipationNode* bro = (ParticipationNode*)(getModuleByPath(nodePath.c_str()));
        bro->ReceiveTransaction(txn);
    }
}


void ParticipationNode::HandleVote(Vote& ReceivedVote)
{
    //macro for methods called directly from foreign nodes (direct memory write optimization)
    Enter_Method_Silent("HandleVote");

    //Ignore invalid vote
    if (!VerifyVote(ReceivedVote))
        return;  //here we could also take actions to disconnect from a potentially malicious peer

    bool IsEquivocation = false;
    if (AddressToVoteMap[ReceivedVote.s][CurrentPeriodSlot].count(ReceivedVote.I))
    {
        //check if repeated vote (already observed)
        //LAS SPECS dicen que si s=0 lo ignoro. Si s=/=0 no lo ignoro?? que hago entonces con repeated votes?
        if (AddressToVoteMap[ReceivedVote.s][CurrentPeriodSlot][ReceivedVote.I][0] == ReceivedVote || AddressToVoteMap[ReceivedVote.s][CurrentPeriodSlot][ReceivedVote.I][1] == ReceivedVote)
            return;
//
        //Equivocation in a proposal step is not allowed
        if (ReceivedVote.s != 1)
            return;
//
//        //Second equivocations are not allowed
//        if (AddressToVoteMap[ReceivedVote.s][0][ReceivedVote.I][1].weight != 0)
//            return;
//
        IsEquivocation = true;
    }

    if (ReceivedVote.r < round) return;
    if (ReceivedVote.r == round+1 && (ReceivedVote.p > 0 || (ReceivedVote.s > 3 && ReceivedVote.s < 253))) return;
    if (ReceivedVote.r == round && (
            (ReceivedVote.p < period==0?0:period-1 || ReceivedVote.p > period+1) ||
            (ReceivedVote.p == period+1 && (ReceivedVote.s > 3 && ReceivedVote.s < 253)) ||
            (ReceivedVote.p == period && (ReceivedVote.s > 3 && ReceivedVote.s < 253) && (ReceivedVote.s < step==0?0:step-1 || ReceivedVote.s > step+1)) ||
            (ReceivedVote.p == period-1 && (ReceivedVote.s > 3 && ReceivedVote.s < 253) && (ReceivedVote.s < lastConcludingStep==0?0:lastConcludingStep-1 || ReceivedVote.s > lastConcludingStep+1)) )) 
            return;


    //load vote into address map
    if (!IsEquivocation)
        AddressToVoteMap[ReceivedVote.s][CurrentPeriodSlot][ReceivedVote.I][0] = ReceivedVote;
    else
    {
//        AddressToVoteMap[ReceivedVote.s][0][ReceivedVote.I][1] = ReceivedVote;
//        CombinedEquivocationWeight[ReceivedVote.s][ReceivedVote.p%2] += ReceivedVote.weight;

        //check if the closest bundle for the given step passes the threshold
        //if it does, it's found
        //could there be more than one CLOSEST BUNDLE and so we'd have two commitable bundles? posible ataque?
//        if (ClosestBundle[ReceivedVote.s]->weight)

        return;
    }


    //proposal votes have no bundles
    if (ReceivedVote.s == 0)
    {
        //save proposal vote somewhere?
        ProposalVotes.push_back(ReceivedVote);

        if (MuValue == EMPTY_PROPOSAL_VALUE || ( ReceivedVote.v.Cached.lowestComputedHash < MuValue.Cached.lowestComputedHash))
        {
            MuValue = ReceivedVote.v;
        }
        return;
    }


    //if active bundle is finished, move to finished bundles
    Bundle* ActiveBundlesRef = &ActiveBundles[CurrentPeriodSlot][ReceivedVote.v.d][ReceivedVote.s];
    ActiveBundlesRef->votes.push_back(ReceivedVote);
    ActiveBundlesRef->weight += ReceivedVote.weight;

    if (ActiveBundlesRef->weight /*+ CombinedEquivocationWeight[ReceivedVote.p][ReceivedVote.s]*/ >= CommitteeThreshold(ReceivedVote.s))
    {
        Bundle* FinishedBundlesRef = &FinishedBundles[CurrentPeriodSlot][ReceivedVote.v.d][ReceivedVote.s];
        FinishedBundlesRef->votes.swap(ActiveBundlesRef->votes);
        FinishedBundlesRef->weight = ActiveBundlesRef->weight;
        ActiveBundlesRef->weight = 0;
//      ActiveBundlesRef->votes.clear();


        if (ReceivedVote.s == 1)
        {
            //if it was a soft vote, I just completed a soft bundle
            FreshestBundle = FinishedBundlesRef;
            SigmaBundle = FinishedBundlesRef;
            if (IsCommitable(ReceivedVote.v))
            {
                uint64_t TotalStake = TotalStakedAlgos();
                for (Account& a : onlineAccounts)
                {
                    VRFOutput credentials;
                    uint64_t accWeight = Sortition(a, TotalStake, CommitteeSize(2), credentials, 2);

                    if (accWeight > 0)
                    {
                        Vote voteToCast(a.I, round, period, 2, ReceivedVote.v, credentials, accWeight);
                        Broadcast(voteToCast);
                    }
                }
            }
        }
        else if (ReceivedVote.s == 2)
        {
            //if it was a certification vote, I just completed a cert bundle
            ConfirmBlock(ReceivedVote.v.d);

            //TESTING
            UpdateBalances();

            GarbageCollectStateForNewRound();
            StartNewRound();

            EV << "COMPLETED ROUND: " << round << ", BY COMMITING BLOCK WITH HASH: " << ReceivedVote.v.d << endl;
        }
        else
        {
            //completed a next bundle or a late/redo/down bundle
            if (ReceivedVote.v != EMPTY_PROPOSAL_VALUE) pinnedValue = ReceivedVote.v;
            else if (Sigma() != EMPTY_PROPOSAL_VALUE) pinnedValue = Sigma();


            //start new period
            GarbageCollectStateForNewPeriod(ReceivedVote.p+1);
            StartNewPeriod(ReceivedVote.p);

            EV << "STARTED NEW PERIOD: " << period << ", WITH PINNED VALUE: " << ReceivedVote.v.d << endl;
        }
    }
}


void ParticipationNode::HandleProposal(ProposalPayload& ReceivedPP)
{
    //validate proposal
    //TODO

    if (CachedFullProposals[CurrentPeriodSlot].count(ReceivedPP.Cached.d))
        return;

    P.push_back(ReceivedPP);
    CachedFullProposals[CurrentPeriodSlot][ReceivedPP.Cached.d] = ReceivedPP.e;
}


void ParticipationNode::HandleBundle(Bundle& ReceivedBundle)
{
    //TODO: validate bundle
    if (!VerifyBundle(ReceivedBundle) || ReceivedBundle.votes[0].r != round || ReceivedBundle.votes[0].p+1 < period)
        return;

    for(Vote& vt : ReceivedBundle.votes)
        HandleVote(vt);
}




void ParticipationNode::GarbageCollectStateForNewRound()
{
    cancelEvent(TimeoutEvent);
    cancelEvent(FastResyncEvent);
    
    pinnedValue = EMPTY_PROPOSAL_VALUE;

    //extra stuff to re-initialize
    FreshestBundle = nullptr;
    SigmaBundle = nullptr;
    MuValue = EMPTY_PROPOSAL_VALUE;
    ActiveBundles[0].clear();
    ActiveBundles[1].clear();
    ActiveBundles[2].clear();
    FinishedBundles[0].clear();
    FinishedBundles[1].clear();
    FinishedBundles[2].clear();

    for (int i = 0; i < 3; i++) FastRecoveryVotes[i].clear();

    ProposalVotes.clear();
    P.clear();
    for (int i = 0; i < 3; i++) CachedFullProposals[i].clear();

    for (int i = 0; i < 256; i++)
        for (int h = 0; h < 3; h++)
            AddressToVoteMap[i][h].clear();

    CurrentPeriodSlot = 0;
}


void ParticipationNode::GarbageCollectStateForNewPeriod(uint64_t NewPeriod)
{
    cancelEvent(TimeoutEvent);
    cancelEvent(FastResyncEvent);

    ProposalVotes.clear();
    P.clear();

    //extra stuff to re-initialize
    SigmaBundle = nullptr;
    MuValue = EMPTY_PROPOSAL_VALUE;

    //TODO: feo. Ver como estructurar esto mas legible
    //basicamente, si el periodo nuevo es period+2, borro lo de period tambien (ya no lo necesito)
    //si el periodo nuevo es mas grande que period+2, borro todo

    ActiveBundles[GetPrevPeriodSlot()].clear();
    FinishedBundles[GetPrevPeriodSlot()].clear();
    CachedFullProposals[GetPrevPeriodSlot()].clear();
    for (int i = 0; i < 256; i++)
        AddressToVoteMap[i][GetPrevPeriodSlot()].clear();

    if (NewPeriod >= period+2)
    {
        ActiveBundles[CurrentPeriodSlot].clear();
        FinishedBundles[CurrentPeriodSlot].clear();
        CachedFullProposals[CurrentPeriodSlot].clear();
        for (int i = 0; i < 256; i++)
            AddressToVoteMap[i][CurrentPeriodSlot].clear();
    }

    if (NewPeriod > period+2)
    {
        ActiveBundles[GetNextPeriodSlot()].clear();
        FinishedBundles[GetNextPeriodSlot()].clear();
        CachedFullProposals[GetNextPeriodSlot()].clear();
        for (int i = 0; i < 256; i++)
            AddressToVoteMap[i][GetNextPeriodSlot()].clear();
    }

    CurrentPeriodSlot = GetNextPeriodSlot();
}


void ParticipationNode::StartNewRound()
{
    lastConcludingStep = step;
   
    period = 0;
    step = 0;
    round++;

    TimeoutEvent->setKind(0);
    scheduleAfter(0, TimeoutEvent);
}


void ParticipationNode::StartNewPeriod(uint64_t FinishedPeriod)
{
    lastConcludingStep = step;
    step = 0;

    period = FinishedPeriod+1;

    TimeoutEvent->setKind(0);
    scheduleAfter(0, TimeoutEvent);
}


ProposalValue ParticipationNode::Sigma()
{
    if (SigmaBundle) return SigmaBundle->votes[0].v;
    else return EMPTY_PROPOSAL_VALUE;
}


ProposalValue ParticipationNode::Mu()
{
    return MuValue;
}


uint256_t ParticipationNode::ComputeLowestCredValue(VRFOutput& Credential, uint64_t weight)
{
    //faking it, but its min(H(VRFCredential || 0...j-1))
//    int randomIndex = rand() % weight;

    uint256_t lowestObservedVal = uint256_t(-1);
    //generator.seed(0);
    //generator.seed(Credential.VRFHash);
    for (int i = 0; i < weight; i++)
    {
        uint256_t HashValue = generator();
        if (HashValue < lowestObservedVal) lowestObservedVal = HashValue;
    }
    return lowestObservedVal;
}


uint256_t ParticipationNode::ComputeBlockHash(LedgerEntry& e)
{
    //TODO: find a CHEAP way to link block structure to computed hash
//    if (e.Txns.size() == 0)
//        return 0;
//
//    uint256_t seed = 0;
//    if (e.Txns.size() <= 1)
//        seed = e.Txns[0].txnID;
//
//    for (int i = 1; i < e.Txns.size(); i++)
//        seed = seed ^ e.Txns[i].txnID;
//
//    BlockHashGenerator.seed(int(seed));
//    return BlockHashGenerator();

    return rand();
}


void ParticipationNode::BlockProposal(LedgerEntry& e)
{
  uint64_t TotalStake = TotalStakedAlgos();

  ProposalValue chosenProposal;
  uint256_t lowestCredHashValue = uint256_t(-1);

  uint256_t blockHash = ComputeBlockHash(e);

  for (Account& a : onlineAccounts)
  {
      VRFOutput credentials;
      uint64_t accWeight = Sortition(a, TotalStake, CommitteeSize(0), credentials, 0);

      if (accWeight > 0)
      {
          uint256_t accLowestHash = ComputeLowestCredValue(credentials, accWeight);
          if (lowestCredHashValue > accLowestHash)
          {
              lowestCredHashValue = accLowestHash;

              chosenProposal.I_orig = a.I;
              chosenProposal.p_orig = period;
              chosenProposal.d = blockHash;
              chosenProposal.Cached.Credentials = credentials;
              chosenProposal.Cached.weight = accWeight;
              chosenProposal.Cached.lowestComputedHash = lowestCredHashValue;
          }
      }
  }

  if (chosenProposal != EMPTY_PROPOSAL_VALUE)
  {
      Vote vt(chosenProposal.I_orig, round, period, 0, chosenProposal, chosenProposal.Cached.Credentials, chosenProposal.Cached.weight);
      Broadcast(vt);

      ProposalPayload pp(e);
      pp.I_orig = chosenProposal.I_orig;
      pp.p_orig = chosenProposal.p_orig;
      pp.Cached.d = chosenProposal.d;
      Broadcast(pp);
  }

  EV << "PROPOSAL FINISHED";
}


void ParticipationNode::SoftVote()
{
    uint64_t TotalStake = TotalStakedAlgos();
    ProposalValue v = Mu();

    if (v != EMPTY_PROPOSAL_VALUE)
    {
        if(v.p_orig == period || FinishedBundles[CurrentPeriodSlot].count(v.d)) //&& FinishedBundles[CurrentPeriodSlot].votes[0].p == period-1) )  //seria lo mismo que pinnedValue == v? No, porque es especificamente p-1 (y no p-i)
            for (Account& a : onlineAccounts)
            {
                VRFOutput credentials;
                uint64_t accWeight = Sortition(a, TotalStake, CommitteeSize(1), credentials, 1);
                if (accWeight == 0) continue;

                Vote voteToCast(a.I, round, period, 1, v, credentials, accWeight);
                Broadcast(voteToCast);
            }
    }
    else if (pinnedValue != EMPTY_PROPOSAL_VALUE && FinishedBundles[GetPrevPeriodSlot()].count(pinnedValue.d) && !FinishedBundles[GetPrevPeriodSlot()].count(EMPTY_PROPOSAL_VALUE.d))
    {
        for (Account& a : onlineAccounts)
        {
            VRFOutput credentials;
            uint64_t accWeight = Sortition(a, TotalStake, CommitteeSize(1), credentials, 1);
            if (accWeight == 0) continue;

            Vote voteToCast(a.I, round, period, 1, pinnedValue, credentials, accWeight);
            Broadcast(voteToCast);
        }
    }
}


void ParticipationNode::NextVote()
{
    //attempt resync first
    ResynchronizationAttempt();


    uint64_t TotalStake = TotalStakedAlgos();

    for (Account& a : onlineAccounts)
    {
        VRFOutput credentials;
        uint64_t accWeight = Sortition(a, TotalStake, CommitteeSize(step), credentials, step);
        if (accWeight == 0) continue;

        Vote voteToCast(a.I, round, period, step, Sigma(), credentials, accWeight);
        if (!IsCommitable(voteToCast.v))
        {
            if (FinishedBundles[CurrentPeriodSlot].count(EMPTY_PROPOSAL_VALUE.d) == 0 && FinishedBundles[CurrentPeriodSlot].count(pinnedValue.d) > 0)
            {
//                //TODO: chequear que period deberia ser p-1. Sino tambien hay algo muy mal
//                if (FinishedBundles[pinnedValue.d][0].votes.size() ||
//                    FinishedBundles[pinnedValue.d][1].votes.size() ||
//                    FinishedBundles[pinnedValue.d][2].votes.size())
//                    EV << "QUE PASO ACA!!!!!! Nunca podria ser 0, 1 o 2 el step. Algo esta muy mal :/";

                voteToCast.v = pinnedValue;
            }
            else voteToCast.v = EMPTY_PROPOSAL_VALUE;
        }

        Broadcast(voteToCast);
    }

}


void ParticipationNode::FastRecovery()
{
    uint64_t TotalStake = TotalStakedAlgos();
    ProposalValue v = Sigma();
    bool bLateCondition = IsCommitable(v);
    bool bRedoCondition = FinishedBundles[(period-1)%2].count(EMPTY_PROPOSAL_VALUE.d) == 0 && FinishedBundles[(period-1)%2].count(pinnedValue.d) > 0;
    bool bDownCondition = !bLateCondition && !bRedoCondition;

    for (Account& a : onlineAccounts)
    {
        VRFOutput credentials;
        
        //late vote stuff and broadcast of observed votes
        uint64_t accWeight = Sortition(a, TotalStake, CommitteeSize(253), credentials, 253);
        if (accWeight > 0)
        {
            Vote voteToCast(a.I, round, period, 253, v, credentials, accWeight);
            if (bLateCondition) Broadcast(voteToCast);
            for (auto& vt : FastRecoveryVotes[0])
            {
                voteToCast.v = vt.v;
                Broadcast(voteToCast);
            }

            lastConcludingStep = 253;
        }

        //redo vote stuff and broadcast of observed votes
        accWeight = Sortition(a, TotalStake, CommitteeSize(254), credentials, 254);
        if (accWeight > 0)
        {
            Vote voteToCast(a.I, round, period, 254, pinnedValue, credentials, accWeight);
            if (bRedoCondition) Broadcast(voteToCast);
            for (auto& vt : FastRecoveryVotes[1])
            {
                voteToCast.v = vt.v;
                Broadcast(voteToCast);
            }

            lastConcludingStep = 254;
        }

        //down vote stuff and broadcast of observed votes
        accWeight = Sortition(a, TotalStake, CommitteeSize(255), credentials, 255);
        if (accWeight > 0)
        {
            Vote voteToCast(a.I, round, period, 255, EMPTY_PROPOSAL_VALUE, credentials, accWeight);
            if (bRedoCondition) Broadcast(voteToCast);
            for (auto& vt : FastRecoveryVotes[2])
            {
                voteToCast.v = vt.v;
                Broadcast(voteToCast);
            }

            lastConcludingStep = 255;
        }
    }

    //broadcast all late, redo and down votes observed (ya lo estoy haciendo, habria que ver si es correcto)
    //TODO: COMO hago esto? Que hago con sortition aca? Lo hago una vez por nodo, una vez por cuenta,una vez por miembro de comite?
    //el voto que broadcasteo lo tengo que pesar segun la cuenta?
}




bool ParticipationNode::IsCommitable(ProposalValue& v)
{
    ProposalValue candidate = Mu();
    if (Sigma() == candidate && candidate == v) return true;
    else return false;
}


void ParticipationNode::ResynchronizationAttempt()
{
    //VER! tema sortition aca...deberia correr esto por cuenta?
    if(FreshestBundle && FreshestBundle->votes.size())
    {
        Broadcast(*FreshestBundle);
        if (FreshestBundle->votes[0].v != EMPTY_PROPOSAL_VALUE && CachedFullProposals[CurrentPeriodSlot].count(FreshestBundle->votes[0].v.d))
        {
            //la reconstruyo y refirmo, o la mando tal cual esta, o como hago? Conviene mas guardarme la pp que la proposal? ver
//            Broadcast(pp);
        }
    }
}


//void ParticipationNode::HandleProposal(ProposalPayload& ReceivedProposal)
//{

//     ProposalValue v;
//     //find proposal value for ledger entry
//     for (auto& val : ObservedProposals)
//         if (val.BlockDigest == ReceivedProposal.CachedBlockDigest)
//         {    
//             v = val;
//             break;
//         }


//     //already observed this proposal, discard
//     if (ObservedProposals.contains(ReceivedProposal))
//             return;

//     //observe proposal
//     ObservedProposals.insert(ReceivedProposal);
    
//     //Relay proposal
//     //TODO. Send to everybody except OriginalProposer and myself, and maybe sender? Do I have the data of who sent it to me?

//     if (IsCommitable(v) && step < cert)
//}


// void ParticipationNode::AddGenesisBlock()
// {
//     LedgerEntry e;
//     //e.SeedAndProof.Seed = "hgyurteydhsjaskeudiaoapdlfkruiu9";
//     Ledger.Entries.push_back(e);
//     round = 1;
// }


 #if SIMULATE_VRF
 VRFOutput ParticipationNode::SimulateVRF()
 {
     VRFOutput Out;
     uint256_t randomHash = generator();

     //export_bits(randomHash, uchar_ptr(Out.VRFHash), 256);
     export_bits(randomHash, Out.VRFHash, 256);

     //get a proof that is just going to be either 0 or 1
     //we are abstracting away the VRF computation, to validate
     //other parts of the protocol
     //Out.VRFProof = std::to_string(0).c_str();
     Out.VRFProof[79] = (unsigned char)(1);

     return Out;
 }
 #endif


// void ParticipationNode::DeriveSeed(stSeedAndProof& SeedAndProof, Account& a, unsigned int period, unsigned int round)
// {
//     unsigned char* PrevSeed = nullptr; //uchar_ptr(Ledger.SeedLookup(currentRound - SeedLookback));
//     unsigned char alpha[crypto_hash_sha256_BYTES];

//     //compute seed proof and preliminary hash
//     if (period == 0)
//     {
//         VRFOutput VRFOutSeed = RunVRF(a, PrevSeed, 32);
//         std::memcpy(&SeedAndProof.Seed, &VRFOutSeed.VRFHash[0], VRFOutSeed.VRFHash.length()); //VER. Por ahora, me quedo con los 32 bytes menos significativos
//         std::memcpy(&SeedAndProof.Proof, &VRFOutSeed.VRFProof[0], VRFOutSeed.VRFProof.length());

//         std::string VRFConcatAddress = std::string(&VRFOutSeed.VRFHash[0], &VRFOutSeed.VRFHash[64]); //+ std::to_string(a.AccountAddress);
//         Hash_SHA256(alpha, (unsigned char*)(VRFConcatAddress.c_str()), VRFConcatAddress.size());
//     }
//     else
//     {
//         ///SeedAndProof.Proof = {0};
//         Hash_SHA256(alpha, PrevSeed, 32);
//     }

//     //compute actual seed
//     if (round % SeedLookback*SeedRefreshInterval < SeedLookback)
//     {
//         //std::string alphaConcatDigestLookup = std::string((char*)(alpha)) + std::string((char*)(Ledger.DigestLookup(round - SeedLookback*SeedRefreshInterval)));
//         //DIGEST LOOKUP NO IMPLEMENTADA! Por ahora:
//         std::string alphaConcatDigestLookup = std::string((char*)(alpha)) + std::string("00000000000000000000000000000000");

//         Hash_SHA256((unsigned char*)(SeedAndProof.Seed.c_str()), (unsigned char*)(alphaConcatDigestLookup.c_str()), alphaConcatDigestLookup.size());
//     }
//     else Hash_SHA256((unsigned char*)(SeedAndProof.Seed.c_str()), alpha, 32);




//     delete[] PrevSeed;
// }


// VRFOutput ParticipationNode::RunVRF(Account& a, unsigned char* bytes, uint64_t bytesLen)
// {
//     VRFOutput cryptoDigest;

//     crypto_vrf_prove(uchar_ptr(cryptoDigest.VRFProof), a.VRFKeys.VRFPrivKey, bytes, bytesLen);
//     //retorna -1 si hay error decodificando la llave secreta, 0 si OK
//     crypto_vrf_proof_to_hash(uchar_ptr(cryptoDigest.VRFHash), uchar_ptr(cryptoDigest.VRFProof));

//     return cryptoDigest;
// }


// bool ParticipationNode::VerifyVRF(Account& a, unsigned char* bytes, uint64_t bytesLen, VRFOutput& HashAndProof)
// {
//     return crypto_vrf_verify(uchar_ptr(HashAndProof.VRFHash), a.VRFKeys.VRFPubKey, uchar_ptr(HashAndProof.VRFProof), bytes, bytesLen) == 0;
// }


 uint64_t ParticipationNode::sortition_binomial_cdf_walk(double n, double p, double ratio, uint64_t money)
 {
   boost::math::binomial_distribution<double> dist(n, p);
   for (uint64_t j = 0; j < money; j++)
   {
       // Get the cdf
       double boundary = cdf(dist, j);

       // Found the correct boundary, break
       if (ratio <= boundary) return j;
   }
   return money;
 }


 uint64_t ParticipationNode::Sortition(Account& a, uint64_t totalMoney, double expectedSize, VRFOutput& cryptoDigest, short Step)
 {
    //  std::string seed = "00000000000000000000============";
    //  std::string role = std::to_string(Step);
    //  std::string m = seed + role;

 #if SIMULATE_VRF
     cryptoDigest = SimulateVRF();
 #else
     cryptoDigest = RunVRF(a, (unsigned char*)(m.c_str()), m.length());
 #endif


     double binomialN = double(a.Money);
     double binomialP = expectedSize / double(totalMoney);

     //BigInt t = byte_array_to_cpp_int(uchar_ptr(cryptoDigest.VRFHash), 64);
     BigInt t = byte_array_to_cpp_int(cryptoDigest.VRFHash, 64);

     double ratio = (BigFloat(t) / two_to_the_hashlen).convert_to<double>();

     return sortition_binomial_cdf_walk(binomialN, binomialP, ratio, a.Money);
 }


uint64_t ParticipationNode::TotalStakedAlgos()
{
     return uint64_t(TOTAL_NODES) * uint64_t(START_MONEY) * uint64_t(TOTAL_ACCOUNTS);
}


#if SIMPLIFIED_BLOCKS
LedgerEntry ParticipationNode::BlockAssembly()
{
    LedgerEntry e;
    e.Txns.swap(TransactionPool);

    return e;
}


void ParticipationNode::ConfirmBlock(uint256_t fakeEntry)
{
    //SI no tengo la proposal, lo tengo que resolver aca


    //TODO: agregar bloques de verdad
    //Ledger.Entries.push_back(hblock);
    Ledger.SimEntries.push_back(fakeEntry);
}
#else
LedgerEntry ParticipationNode::BlockAssembly()
{
    auto e = LedgerEntry();
    auto n_tp = TransactionPool.size();

    std::move(TransactionPool.rbegin()+(n_tp>10?10:n_tp), TransactionPool.rbegin(), e.Txns.begin());

    return LedgerEntry();
}


void ParticipationNode::ConfirmBlock(LedgerEntry& e)
{
     //TODO: agregar bloques de verdad
     Ledger.Entries.push_back(e);
}
#endif




void ParticipationNode::ReceiveTransaction(Transaction& txn)
{
    TransactionPool.push_back(txn);
}


bool ParticipationNode::VerifyVote(Vote& vt)
{
    return true;
}


bool ParticipationNode::VerifyBundle(Bundle& b)
{
    return true;
}

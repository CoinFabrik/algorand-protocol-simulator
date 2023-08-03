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

/**********************************************************************************/
//PROTOCOL PARAMETERS (from abft.pdf in specs)
unsigned int SeedLookback = 2;
unsigned int SeedRefreshInterval = 80;
unsigned int BalanceLookback = 2*SeedLookback*SeedRefreshInterval;

//time parameters (in seconds)
float Lambda = 2.f;
float Lambda0 = 1.7f;
float LambdaF = 300.f; //5 min
float UppercaseLambda = 17.f;

inline float FilterTimeout(unsigned int p){return 2.f * (p==0? Lambda0 : Lambda);}

//useful constants
ProposalValue EMPTY_PROPOSAL_VALUE;
Address FEE_SINK = 0;
/**********************************************************************************/


int ParticipationNode::MaxNodeID = 0;


ParticipationNode::ParticipationNode() : round(1), period(0), step(0), lastConcludingStep(0)
{
    FastResyncEvent = new cMessage(nullptr, 253);
    TimeoutEvent = new cMessage(nullptr, 0);

    FreshestBundle = nullptr;
    SigmaBundle = nullptr;
    MuValue = EMPTY_PROPOSAL_VALUE;
    pinnedValue = EMPTY_PROPOSAL_VALUE;

//    GlobalSimulationManager::SimManager->Network.ParticipationNodes[NodeID] = this;
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


    //initialize relay connections. Por ahora, 4 relays random
    for (int i = 0; i < 4; i++)
    {
        int RelayToConnect = rand() % RELAYS;
        while(std::find(RelayConnections.begin(), RelayConnections.end(), RelayToConnect) != RelayConnections.end()) RelayToConnect = rand() % RELAYS;

        RelayConnections.push_back(RelayToConnect);
    }

    //update sim manager network relay and part node data accordingly
    GlobalSimulationManager::SimManager->Network.LoadPartNode(this, this->getIndex(), RelayConnections);
}


void ParticipationNode::InitOnlineAccounts()
{
    for (int i = getIndex()*10; i < (getIndex()+1)*10; i++)
    {
        auto addr = i;
        auto NewAccount = Account(addr, GlobalSimulationManager::SimManager->BalanceMap[addr].RawBalance);
        onlineAccounts.push_back(NewAccount);
    }
//    for (uint64_t i = 0; i < uint64_t(TOTAL_ACCOUNTS); i++)
//    {
//        auto NewAccount = Account(rand(), uint64_t(START_MONEY));
//        onlineAccounts.push_back(NewAccount);
//
//        //initialize global balance tracker
//        GlobalSimulationManager::SimManager->BalanceMap[NewAccount.I] =  BalanceRecord(NewAccount.Money, true);
//    }
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

    //get transactions from last addition to the ledger
    auto& LastBlock = *(Ledger.Entries.end()-1);

    //update all balances according to last block
    for (Transaction* txn : LastBlock.Txns)
    {
        //por ahora solo pay
        GlobalSimulationManager::SimManager->BalanceMap[txn->Sender].RawBalance -= txn->Fee + txn->Amount;
        GlobalSimulationManager::SimManager->BalanceMap[txn->Receiver].RawBalance += txn->Amount;
//        Balance[FEE_SINK_ADDRESS].RawBalance += txn.Fee;
    }

    if (Ledger.Entries.size()>BalanceLookback)
    {
        //updatear all cached balances according to lookback block
        auto& LookbackBlock =  *(Ledger.Entries.end() - 1 - BalanceLookback);
        for (Transaction* txn : LookbackBlock.Txns)
        {
            //por ahora solo pay
            GlobalSimulationManager::SimManager->BalanceMap[txn->Sender].OldBalance -= txn->Fee + txn->Amount;
            GlobalSimulationManager::SimManager->BalanceMap[txn->Receiver].OldBalance += txn->Amount;
        }
    }
}


void ParticipationNode::SimulateTransactions()
{
    //for now, generate a random number of random txns (max 1 per managed account)
    int nTxns = rand()%2; //onlineAccounts.size();
    for (int i = 0; i < nTxns; i++)
    {
        Transaction txn = GenerateRandomTransaction();
        Broadcast((void*)(&txn), TXN);
    }
}


Transaction ParticipationNode::GenerateRandomTransaction()
{
    Transaction txn;
    txn.txnID = GlobalSimulationManager::SimManager->GetNextTxnID();
    txn.type = Transaction::PAY;  //por ahora solo pay
    //pick random sender (implementar lista de addresses...ninguna de las dos cuentas tiene por que estar online. Separar managed de online)
    txn.Sender = onlineAccounts[rand()%onlineAccounts.size()].I;
    txn.Receiver = onlineAccounts[rand()%onlineAccounts.size()].I;
    txn.Amount = rand() % GlobalSimulationManager::SimManager->BalanceMap[txn.Sender].RawBalance;  //es el actual o el de 320 rounds prior? repasar eso
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
            OUT_LOG_STEP_EVENT();

            //schedule a soft vote
            TimeoutEvent->setKind(1);
            scheduleAfter(FilterTimeout(period), TimeoutEvent);
            //schedule a fast recovery attempt
            scheduleAfter(LambdaF + 0, FastResyncEvent);

            BlockProposal(e);

            lastConcludingStep = 0;
            step = 1;
            OUT_LOG_STEP_EVENT();
            //aca ya propuse, y me quedo esperando el soft
        }
        else if (msg->getKind() == 1)
        {
            step = 1;
            OUT_LOG_STEP_EVENT();


            TimeoutEvent->setKind(3);
            scheduleAt(startTime + fmax(4.f * Lambda, UppercaseLambda), TimeoutEvent);

            SoftVote();

            step = 2;
            OUT_LOG_STEP_EVENT();

            lastConcludingStep = 1;

            //certification vote happens from now on, but is driven by messages (not timeout)
            //node has to observe a cert bundle BEFORE the next timeout
        }
        else if (msg->getKind() < 253)  // >= 3 por definicion
        {
            lastConcludingStep = step;
            step = msg->getKind();

            OUT_LOG_STEP_EVENT();

            //252 is the last next-vote. After this, its periodic cycles of late, recovery, redo
            if (msg->getKind() <= 251)
            {
                TimeoutEvent->setKind(step + 1);
                scheduleAt(startTime + fmax(4.f * Lambda, UppercaseLambda) + pow(2.f, step) * Lambda + 0, TimeoutEvent);
            }

            NextVote();
        }
        else if(msg->getKind() <= 255)
        {
            lastConcludingStep = step;
            step = 255;
            OUT_LOG_STEP_EVENT();

            scheduleAfter(LambdaF + 0, FastResyncEvent);

            FastRecovery();
        }

        else if(msg->getKind() == 256)
        {
            if (!TravelingTxnQueue.empty())
            {
                int sz = TravelingTxnQueue.size();
                HandleTransaction(TravelingTxnQueue.back());
                if (TravelingTxnQueue.size() == sz) TravelingTxnQueue.pop_back();
            }
            cancelAndDelete(msg);
        }
        else if(msg->getKind() == 257)
        {
            if (!TravelingVoteQueue.empty())
            {
                int sz = TravelingVoteQueue.size();
                HandleVote(TravelingVoteQueue.back());
                if (TravelingVoteQueue.size() == sz) TravelingVoteQueue.pop_back();
            }
            cancelAndDelete(msg);
        }
        else if(msg->getKind() == 258)
        {
            if (!TravelingProposalQueue.empty())
            {
                int sz = TravelingProposalQueue.size();
                HandleProposal(TravelingProposalQueue.back());
                if (TravelingProposalQueue.size() == sz) TravelingProposalQueue.pop_back();
            }
            cancelAndDelete(msg);
        }
        else if(msg->getKind() == 259)
        {
            if (!TravelingBundleQueue.empty())
            {
                int sz = TravelingBundleQueue.size();
                HandleBundle(TravelingBundleQueue.back());
                if (TravelingBundleQueue.size() == sz) TravelingBundleQueue.pop_back();
            }
            cancelAndDelete(msg);
        }
    }
}


void ParticipationNode::Broadcast(void* data, MsgType type)
{
    GlobalSimulationManager::SimManager->PropagateMessageThroughNetwork(RelayConnections, getIndex(), data, type);
}


void ParticipationNode::TEST_ScheduleTxnHandling(float delay, Transaction* txn)
{
    Enter_Method_Silent("TEST_ScheduleTxnHandling");

    if (delay==0.f){HandleTransaction(txn); return;}

    TravelingTxnQueue.push_back(txn);
    scheduleAfter(delay, new cMessage(nullptr, 256));
}


void ParticipationNode::TEST_ScheduleVoteHandling(float delay, Vote& vt)
{
    Enter_Method_Silent("TEST_ScheduleVoteHandling");

    if (delay==0.f){HandleVote(vt); return;}

    TravelingVoteQueue.push_back(vt);
    scheduleAfter(delay, new cMessage(nullptr, 257));
}


void ParticipationNode::TEST_ScheduleProposalHandling(float delay, ProposalPayload& pp)
{
    Enter_Method_Silent("TEST_ScheduleproposalHandling");

    if (delay==0.f){HandleProposal(pp); return;}

    TravelingProposalQueue.push_back(pp);
    scheduleAfter(delay, new cMessage(nullptr, 258));
}


void ParticipationNode::TEST_ScheduleBundleHandling(float delay, Bundle& b)
{
    Enter_Method_Silent("TEST_ScheduleBundleHandling");

    if (delay==0.f){HandleBundle(b); return;}

    TravelingBundleQueue.push_back(b);
    scheduleAfter(delay, new cMessage(nullptr, 259));
}


void ParticipationNode::HandleTransaction(Transaction* ReceivedTxn)
{
    //macro for methods called directly from foreign nodes (direct memory write optimization)
    Enter_Method_Silent("HandleTransaction");

    //TODO: validate?
    if (TransactionPool.size() < TXN_POOL_LIMIT)
        TransactionPool.push_back(ReceivedTxn);
    else
    {
        //log transaction dropping event
//        EV << "TXN ID: " << ReceivedTxn.txnID << " DROPPED BY " << getIndex() << endl;
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
//        ProposalVotes.push_back(ReceivedVote);

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
                        Broadcast((void*)(&voteToCast), VOTE);
                    }
                }
            }
        }
        else if (ReceivedVote.s == 2)
        {
            //if it was a certification vote, I just completed a cert bundle
//            ConfirmBlock(ReceivedVote.v.d);
            if (CachedFullProposals[CurrentPeriodSlot].count(ReceivedVote.v.d))
                ConfirmBlock(CachedFullProposals[CurrentPeriodSlot][ReceivedVote.v.d]);
            else
            {
                EV << "ACA NO TENGO LA PROPUESTA. TENGO QUE HACER ALGO" << endl;
            }

            //TESTING
            UpdateBalances();

            GarbageCollectStateForNewRound();
            StartNewRound();

//            EV << "COMPLETED ROUND: " << round-1 << ", BY COMMITING BLOCK WITH HASH: " << ReceivedVote.v.d << endl;
        }
        else
        {
            //completed a next bundle or a late/redo/down bundle
            if (ReceivedVote.v != EMPTY_PROPOSAL_VALUE) pinnedValue = ReceivedVote.v;
            else if (Sigma() != EMPTY_PROPOSAL_VALUE) pinnedValue = Sigma();


            //start new period
            GarbageCollectStateForNewPeriod(ReceivedVote.p+1);
            StartNewPeriod(ReceivedVote.p);

//            EV << "STARTED NEW PERIOD: " << period-1 << ", WITH PINNED VALUE: " << ReceivedVote.v.d << endl;
        }
    }
}


void ParticipationNode::HandleProposal(ProposalPayload& ReceivedPP)
{
    //validate proposal
    //TODO

    if (CachedFullProposals[CurrentPeriodSlot].count(ReceivedPP.Cached.d))
        return;

//    P.push_back(ReceivedPP);
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
//    P.clear();
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
//    P.clear();

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

    GlobalSimulationManager::SimManager->NodeStartedNewRound(this, round);
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
      Broadcast((void*)(&vt), VOTE);

      ProposalPayload pp(e);
      pp.I_orig = chosenProposal.I_orig;
      pp.p_orig = chosenProposal.p_orig;
      pp.Cached.d = chosenProposal.d;
      Broadcast((void*)(&pp), PROPOSAL);
  }
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
                Broadcast((void*)(&voteToCast), VOTE);
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
            Broadcast((void*)(&voteToCast), VOTE);
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

        Broadcast((void*)(&voteToCast), VOTE);
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
            if (bLateCondition) Broadcast((void*)(&voteToCast), VOTE);
            for (auto& vt : FastRecoveryVotes[0])
            {
                voteToCast.v = vt.v;
                Broadcast((void*)(&voteToCast), VOTE);
            }

            lastConcludingStep = 253;
        }

        //redo vote stuff and broadcast of observed votes
        accWeight = Sortition(a, TotalStake, CommitteeSize(254), credentials, 254);
        if (accWeight > 0)
        {
            Vote voteToCast(a.I, round, period, 254, pinnedValue, credentials, accWeight);
            if (bRedoCondition) Broadcast((void*)(&voteToCast), VOTE);
            for (auto& vt : FastRecoveryVotes[1])
            {
                voteToCast.v = vt.v;
                Broadcast((void*)(&voteToCast), VOTE);
            }

            lastConcludingStep = 254;
        }

        //down vote stuff and broadcast of observed votes
        accWeight = Sortition(a, TotalStake, CommitteeSize(255), credentials, 255);
        if (accWeight > 0)
        {
            Vote voteToCast(a.I, round, period, 255, EMPTY_PROPOSAL_VALUE, credentials, accWeight);
            if (bRedoCondition) Broadcast((void*)(&voteToCast), VOTE);
            for (auto& vt : FastRecoveryVotes[2])
            {
                voteToCast.v = vt.v;
                Broadcast((void*)(&voteToCast), VOTE);
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
        Broadcast((void*)(FreshestBundle), BUNDLE);
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
    return GlobalSimulationManager::SimManager->TotalStakedAlgos;
//    return uint64_t(TOTAL_NODES) * uint64_t(START_MONEY) * uint64_t(TOTAL_ACCOUNTS);
}


#if SIMPLIFIED_BLOCKS
LedgerEntry ParticipationNode::BlockAssembly()
{
    LedgerEntry e;
//    e.Txns.swap(TransactionPool);

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

    auto n_block = 1000;

    for (int i = n_tp-1; i > n_tp - n_block && i>=0; i--)
    {
        e.Txns.push_back(TransactionPool[i]);
    }
//    std::copy(TransactionPool.rbegin()+(n_tp > n_block? n_block : n_tp), TransactionPool.rbegin(), e.Txns.rend());

    return LedgerEntry();
}


void ParticipationNode::ConfirmBlock(LedgerEntry& e)
{
     Ledger.Entries.push_back(e);

     for (Transaction* txn : e.Txns)
     {
         auto ptrEq = [txn](Transaction* t1) { return txn == t1; };

         std::vector<Transaction*>::iterator it = std::find_if(TransactionPool.begin(), TransactionPool.end(), ptrEq);
         if (it != TransactionPool.end())
             TransactionPool.erase(it);
     }
}
#endif


bool ParticipationNode::VerifyVote(Vote& vt)
{
    return true;
}


bool ParticipationNode::VerifyBundle(Bundle& b)
{
    return true;
}

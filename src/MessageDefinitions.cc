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

#include "MessageDefinitions.h"


Register_Class(AlgorandMessage)


void AlgorandMessage::SetCredentials(VRFOutput& Cred, Address& I)
{
    ProposerVRFCredentials.VRFHash = Cred.VRFHash;
    ProposerVRFCredentials.VRFProof = Cred.VRFProof;
}


AlgorandMessage* AlgorandMessage::MakeMessage(std::vector<AlgorandMessage*>& MsgBuffer, uint64_t round, short step, uint64_t votes, VRFOutput& SenderVRFCredentials, Address& SenderAddress, LedgerEntry& Block)
{
    AlgorandMessage* NewMsg = nullptr;
    if (MsgBuffer.size())
    {
        NewMsg = MsgBuffer.back();
        MsgBuffer.pop_back();

        //NewMsg->SetName(std::to_string(Block.PlaceholderID).c_str());
        NewMsg->round = round;
        NewMsg->step = step;
        NewMsg->SetVotes(votes);
        NewMsg->SetCredentials(SenderVRFCredentials, SenderAddress);
        NewMsg->SetPayload(Block);
    }
    else
    {
        NewMsg = new AlgorandMessage(round, step, ""); //std::to_string(Block.PlaceholderID).c_str());
        NewMsg->SetVotes(votes);
        NewMsg->SetCredentials(SenderVRFCredentials, SenderAddress);
        NewMsg->SetPayload(Block);
    }
    return NewMsg;
}


void AlgorandMessage::RecycleMessage(cSimpleModule* callerModule, std::vector<AlgorandMessage*>& MsgBuffer, AlgorandMessage* msg)
{
    if(MsgBuffer.size() < REUSABLE_MSG_BUFFER_SIZE) MsgBuffer.push_back(msg);
    else callerModule->cancelAndDelete(msg);

    msg = nullptr;  //either way, prevent message from being used
}


AlgorandMessage* AlgorandMessage::DuplicateMessage(std::vector<AlgorandMessage*>& MsgBuffer, AlgorandMessage* msg)
{
    return MakeMessage(MsgBuffer, msg->round, msg->step, msg->votes, msg->ProposerVRFCredentials, msg->ProposerAddress, msg->Payload);
}


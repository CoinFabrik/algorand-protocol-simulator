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

#include "RelayNode.h"


Define_Module(RelayNode);


RelayNode::RelayNode():cSimpleModule(4096)
{
    // TODO Auto-generated constructor stub

}

RelayNode::~RelayNode()
{
    for (auto& m : ReusableMessages) cancelAndDelete(m);
}


void RelayNode::initialize()
{

}


void RelayNode::activity()
{
    while(true)
    {
        AlgorandMessage* msg = (AlgorandMessage*)(receive());
        if (msg)
        {
            for (int i = 0; i < gateSize("gate"); i++)
            {
                if (msg->getArrivalGate()->getIndex() != i)
                {
                    send(AlgorandMessage::DuplicateMessage(ReusableMessages, msg), "gate$o", i);
                }
            }
            AlgorandMessage::RecycleMessage(this, ReusableMessages, msg);
        }
    }
}

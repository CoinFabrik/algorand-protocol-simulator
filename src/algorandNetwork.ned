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
//
// TODO documentation

simple ParticipationNode
{
}


simple GlobalSimulationManager
{
}


network AlgorandNetwork
{
    submodules:
        Manager : GlobalSimulationManager;
        PartNode[0]: ParticipationNode;
        
    connections:
//        for i=0..partCount-1, for j=0..relCount-1
//        { 
//        	RelayNode[j].gate++ <--> {  delay = 100ms; } <--> PartNode[i].gate++;
//        	//RelayNode[j].gate++ <--> PartNode[i].gate++;
//        }

        
//        for j=0..relCount-1, for i=0..9
//        {
//        	RelayNode[j].gate++ <--> {  delay = 100ms; } <--> PartNode[j*10 + i].gate++;
//        }
        
//        for j=0..relCount-1
//        {
//            RelayNode[j].gate++ <--> {  delay = 100ms; } <--> RelayNode[(j+1) % (relCount-1)].gate++;
//        }
}


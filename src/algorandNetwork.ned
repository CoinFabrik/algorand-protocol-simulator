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
    gates:
        inout gate[];
}

simple RelayNode
{
    gates:
        inout gate[];
}


network AlgorandNetwork
{
    @display("bgb=806,486");

	int partCount = 1600;
	int relCount = 160;

//	int partCount = 14;
//	int relCount = 2;
	
//	int partCount = 3;
//	int relCount = 1;

    submodules:
        PartNode[partCount]: ParticipationNode;
        RelayNode[relCount]: RelayNode;
        
    connections:
//        for i=0..partCount-1, for j=0..relCount-1
//        { 
//        	RelayNode[j].gate++ <--> {  delay = 50ms; } <--> PartNode[i].gate++;
//        }
        
        for j=0..relCount-1, for i=0..9
        {
        	RelayNode[j].gate++ <--> {  delay = 50ms; } <--> PartNode[j*10 + i].gate++;
        }
        
        for j=0..relCount-1
        {
            RelayNode[j].gate++ <--> {  delay = 50ms; } <--> RelayNode[(j+1) % (relCount-1)].gate++;
        }
}


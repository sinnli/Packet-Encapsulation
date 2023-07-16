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

package lab_2_mid;
import ned.IBidirectionalChannel;

network final
{
    @display("bgb=658,387");
    submodules:
        host[4]: host {
           // @display("p=124,181");
        }
        switch: Switch {

            @display("p=300,156");
        }
    connections:
        host[0].gateToSwitch <--> {  datarate = 100Mbps; } <--> switch.gate++;
        host[1].gateToSwitch <--> {  datarate = 100Mbps; } <--> switch.gate++;
        host[2].gateToSwitch <--> {  datarate = 100Mbps; } <--> switch.gate++;
        host[3].gateToSwitch <--> {  datarate = 100Mbps; } <--> switch.gate++;
}

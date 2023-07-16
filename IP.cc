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

#include "IP.h"
#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "Eth_pck_m.h"
#include "IP_pck_m.h"
#include "App_pck_m.h"
using namespace omnetpp;

Define_Module(IP);

void IP::initialize()
{
    // TODO - Generated method body
    int x = par("host_num");
    std::string s = std::to_string(x);
    myIP = "172.168.32.";
    myIP = myIP+s;
}

void IP::handleMessage(cMessage *msg)
{
    //App msg
    if(msg->getKind() == 3){
        App_pck *app_msg = check_and_cast<App_pck *>(msg);

        //make ip address
        std::string dest = "172.168.32.x";
        do{
            dest.pop_back();
            int x = par("ip_add");
            std::string s = std::to_string(x);
            dest = dest+s;
        }while(dest == myIP);

        //generate ip msg + encapsulate
        IP_pck *ipmsg = new IP_pck();
        ipmsg->setByteLength(20);
        ipmsg->setSource_ip(myIP.c_str());
        ipmsg->setDest_ip(dest.c_str());
        ipmsg->setKind(2);
        ipmsg->encapsulate(app_msg);


        //send
        send(ipmsg,"eth$o");
    }

    //ip msg from eth module
    if(msg->getKind() == 2){
        IP_pck *IP_msg = check_and_cast<IP_pck *>(msg);

        //check appropriate ip address
        if(IP_msg->getDest_ip() == myIP){
            cPacket *payload = IP_msg->decapsulate();
            App_pck* app_msg = check_and_cast<App_pck*>(payload);
            send(app_msg,"app$o");
        }
    }
}

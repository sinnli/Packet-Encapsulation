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

#include "switch.h"


Define_Module(Switch);

void Switch::initialize()
{
    // TODO - Generated method body
    life_time = par("switch_aging_time");
    sLatency = par("swtitchLatencyTime");
    delay_send = 0;
}

void Switch::handleMessage(cMessage *msg)
{

    Eth_pck *eth_msg = check_and_cast<Eth_pck *>(msg);
    if(!msg->isSelfMessage()){

        //ethernet
        if(msg->getKind() == 0){
            delay_send = sLatency*eth_msg->getByteLength();
            arrivalGate = eth_msg->getArrivalGate()->getIndex();
            scheduleAt(simTime() + delay_send, msg);
        }
        else{
           bubble("Error in packet type of the packet got at Switch");
        }
    }

    //self message -> passed latency time
    else{
        //update DataBase
        updateDataBase();

        //Ethernet message
        handleEthMsg(msg);

    }
}

void Switch::handleEthMsg(cMessage *msg){
    //some variables
    std::string temp_source,temp_dest;
    int ttl;

    //check and cast
    Eth_pck *eth_msg = check_and_cast<Eth_pck *>(msg);
    temp_source = eth_msg->getSource_mac();
    temp_dest = eth_msg->getDest_mac();

    //search for the mac address in Data Base
       int ggate = search(temp_source);

       //didn't found the mac address in data base so make new one
       if(ggate == -1){
           append(temp_source,arrivalGate, life_time + simTime().dbl());
       }

       //found this mac address in data base
      else{
          ttl = eth_msg->getTTL();
          eth_msg->setTTL(ttl-1);   //update ttl
          //check if ttl > 0
          if(eth_msg->getTTL() <= 0){
             cancelAndDelete(eth_msg);
             return ;
          }
      }

      //navigate the incoming package
       gateOut = search(temp_dest);

       if(gateOut >= 0){
           cChannel *txchannel = gate("gate$o", gateOut)->getTransmissionChannel();
           simtime_t txFinishTime = txchannel->getTransmissionFinishTime();
           if(txFinishTime >= simTime()){
                txFinishTime = txFinishTime-simTime();
                sendDelayed(eth_msg,txFinishTime,"gate$o",gateOut);
           }
           else{
            send(eth_msg,"gate$o",gateOut);
           }
       }

       //broadcast the message
       else if(gateOut == -1){
           broadcastEth(arrivalGate, eth_msg);
       }
}


void Switch::updateDataBase(){
    struct DataBaseNode *temp =NULL,*todel = NULL,*last = NULL;
    temp = DataBaseHead;


    printf("update and print linked list                     SW \n");
    while(temp!=NULL){
        printf("the mac %s \n",temp->SourceMac.c_str());
        printf("the gate of mac is %d\n",temp->gate);

        if(simTime().dbl() > temp->agingtime ){    //mean life time ended
            //delete the head
            if(temp->SourceMac == DataBaseHead->SourceMac){
                DataBaseHead =temp->next;
                delete temp;

            }

            //delete in the middle
            else if(temp->next != NULL){
                todel = temp;
                temp = todel->next;
                last->next = temp;
                delete todel;
            }

            //delete the end
            else{
                todel = temp;
                last->next = NULL;
                delete todel;
                break;
            }
        }
        last = temp;
        temp = temp->next;
    }
}


int Switch::search(std::string s){
    struct DataBaseNode* current = DataBaseHead;
    while(current != NULL){
        if(current->SourceMac == s)
            return current->gate;
        current = current->next;
    }
    return -1;
}

void Switch::append(std::string mac, int gate, double aging){
    //make new node
    struct DataBaseNode *newEntry  = new struct DataBaseNode();
    newEntry->SourceMac = mac;
    newEntry->gate = gate;
    newEntry->agingtime = aging;
    newEntry->next = NULL;

    struct DataBaseNode *last = DataBaseHead;
    if(DataBaseHead == NULL){
        DataBaseHead = newEntry;
        return;
    }

    while(last->next != NULL)
       last = last->next;

    last->next = newEntry;
    return;
}

void Switch::broadcastEth(int g, Eth_pck* copy){
    for(int i = 0; i<num_of_host; i++){
        Eth_pck *copyy = (Eth_pck *) copy->dup();
        if (i!=g){
                       cChannel *txchannel = gate("gate$o",i)->getTransmissionChannel();
                       if(txchannel->isBusy()){
                           sendDelayed(copyy,gate("gate$o",i)->getTransmissionChannel()->getTransmissionFinishTime()- simTime(),"gate$o",i);
                       }
                       else{
                           send(copyy, "gate$o",i);
                       }
                   }
           }
}



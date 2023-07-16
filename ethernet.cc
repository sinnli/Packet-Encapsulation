//
// This file is part of an OMNeT++/OMNEST simulation example.
//
// Copyright (C) 2003 Ahmet Sekercioglu
// Copyright (C) 2003-2015 Andras Varga
//
// This file is distributed WITHOUT ANY WARRANTY. See the file
// `license' for details on this and other legal matters.
//

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
#include "Eth_pck_m.h"
#include "IP_pck_m.h"
#include "App_pck_m.h"
#include "ARP_m.h"

using namespace omnetpp;

#define num_of_host 3

/**
 * This model is exciting enough so that we can collect some statistics.
 * We'll record in output vectors the hop count of every message upon arrival.
 * Output vectors are written into the omnetpp.vec file and can be visualized
 * with the Plove program.
 *
 * We also collect basic statistics (min, max, mean, std.dev.) and histogram
 * about the hop count which we'll print out at the end of the simulation.
 */
//typedef std::map<std::string, std::pair<std::string,int>> map_type;
/*struct cmp_str{
    bool operator()(char const*a,char const*b) const
    {
        return std::strcmp(a, b)<0;
    }
};
*/

class ethernet : public cSimpleModule
{
  private:
    //std::map<const std::string,std::pair<std::string,int>>::iterator ArpTable;  //<ip,mac,ttl>
    std::string ArpTable[num_of_host][3];  //[ip][mac][ttl]
    std::string EtherAdd;
    std::string myIP;
    IP_pck *save;
    bool waitForAnswer;
    double life_time;


  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void handleEthMsg(Eth_pck *eth_msg);
    virtual void handleArpMsg(ARP *arp_msg);
    virtual void handleIPMsg(IP_pck *IP_msg);
    virtual void updateArpTable();
    virtual void sendMsg(cMessage* msg);

};

Define_Module(ethernet);

void ethernet::initialize()
{
    // Initialize variables
    life_time = par("arp_life_time");
    int x=par("host_num");
    myIP = "172.168.32." + std::to_string(x);
    EtherAdd = "11.11.11.11.11.0" + std::to_string(x);
    waitForAnswer = FALSE;
    for(int i = 0;i<num_of_host;i++){
        for(int j=0;j<3;j++){
            ArpTable[i][j] = "";
        }
    }
}

void ethernet::handleMessage(cMessage *msg)
{
    //ethernet msg (from switch)
    if (msg->getKind() == 0){
        //short Ethertype;             //0: enthernet   1:arp
        Eth_pck *eth_msg = check_and_cast<Eth_pck *>(msg);
        std::string mac = eth_msg->getDest_mac();

        //ethernet message
        if(eth_msg->getEthertype() == 0){
            if(std::strcmp(EtherAdd.c_str(),mac.c_str())==0){
                handleEthMsg(eth_msg);
            }
        }

        //arp message
        else if(eth_msg->getEthertype() == 1){
            //dencapsulate Eth_pck to Arp_pck

            cPacket *payload = eth_msg->decapsulate();
            ARP* arp_msg = check_and_cast<ARP*>(payload);

             if(std::strcmp(EtherAdd.c_str(),mac.c_str())==0){\
                handleArpMsg(arp_msg);
             }
             else if(std::strcmp(mac.c_str(),"ff.ff.ff.ff.ff.ff")==0){
                 handleArpMsg(arp_msg);
            }
        }

    }

    //ip message
    if(msg->getKind() == 2){
     IP_pck *IP_msg = check_and_cast<IP_pck *>(msg);
         handleIPMsg(IP_msg);
    }

}
void ethernet::handleEthMsg(Eth_pck *eth_msg)
{
    cPacket *payload = eth_msg->decapsulate();
    IP_pck* ip_msg = check_and_cast<IP_pck *>(payload);
    send(ip_msg,"ip$o");
}

void ethernet::handleArpMsg(ARP *arp_msg){

    //request
    if(arp_msg->getOpcode() == 1){
      //check if this is my ip
        if(arp_msg->getDest_ip() == myIP){
            //make arp answer
            const char *ethadd = EtherAdd.c_str();
            const char *myip = myIP.c_str();
            //generate answer
            ARP *answer = new ARP();
            answer->setKind(1);
            answer->setByteLength(42);
            answer->setOpcode(2);       //arp reply (answer)
            answer->setTTL(64);
            answer->setSource_mac(ethadd);
            answer->setDest_mac(arp_msg->getSource_mac());
            answer->setSource_ip(myip);
            answer->setDest_ip(arp_msg->getSource_ip());

            Eth_pck *eth_frame = new Eth_pck();
            eth_frame->setByteLength(18);
            eth_frame->setDest_mac(arp_msg->getSource_mac());
            eth_frame->setSource_mac(ethadd);
            eth_frame->setTTL(64);
            eth_frame->setEthertype(1);
            eth_frame->setKind(0);

            eth_frame->encapsulate(answer);

            //send the answer
            sendMsg(eth_frame);
        }
    }

    //reply
    else{
        //update arp table
        updateArpTable();

        for(int i = 0; i<num_of_host;i++){
            if(ArpTable[i][0] ==""){
                ArpTable[i][0] = arp_msg->getSource_ip();
                ArpTable[i][1] = arp_msg->getSource_mac();
                ArpTable[i][2] = std::to_string(life_time + simTime().dbl());
                break;
            }
        }

        //we waited for answer to send an ip msg
        if(waitForAnswer){ //we waited for answer to send an ip msg
            waitForAnswer = FALSE;
            //generate appropriate eth msg
            Eth_pck *msg = new Eth_pck();
            msg->setEthertype(0);
            msg->setDest_mac(arp_msg->getSource_mac());
            msg->setSource_mac(EtherAdd.c_str());
            msg->setTTL(64);
            msg->encapsulate(save);
            sendMsg(msg);
        }
    }

}

void ethernet::handleIPMsg(IP_pck *IP_msg)
{
    //update arp table
    updateArpTable();
    int Found =  0;

    //serach for the mac in the arp table
   for(int i =0 ; i<num_of_host ; i++){

       // we found the mac
       if(ArpTable[i][0].compare(IP_msg->getDest_ip()) == 0){

           //generate ethernet message
           Eth_pck *msg = new Eth_pck();
           msg->setEthertype(0);
           msg->setDest_mac(ArpTable[i][1].c_str());
           msg->setSource_mac(EtherAdd.c_str());
           msg->setTTL(64);
           msg->encapsulate(IP_msg);
           //send it
           sendMsg(msg);
           Found = 1;
           break;
       }
   }

   //didnt find the mac add of this ip
   if(!Found){
    waitForAnswer = TRUE;
    save = IP_msg;

    //make arp request
    const char *ethadd = EtherAdd.c_str();
    const char *myip = myIP.c_str();
    //make arp request
    ARP *req = new ARP();
    req->setKind(1);
    req->setOpcode(1);      //arp requet
    req->setByteLength(42);
    req->setSource_ip(myip);
    req->setSource_mac(ethadd);
    req->setTTL(64);
    req->setDest_ip(IP_msg->getDest_ip());
    req->setDest_mac("ff.ff.ff.ff.ff.ff");

    Eth_pck *eth_frame = new Eth_pck();
    eth_frame->setByteLength(18);
    eth_frame->setDest_mac("ff.ff.ff.ff.ff.ff");
    eth_frame->setSource_mac(ethadd);
    eth_frame->setTTL(64);
    eth_frame->setEthertype(1);
    eth_frame->setKind(0);

    eth_frame->encapsulate(req);

    //send it
    sendMsg(eth_frame);
   }

}

void ethernet::updateArpTable(){
    for(int i = 0; i<num_of_host;i++){
        if(ArpTable[i][2] != ""){                               //check if not empty
            if(std::to_string(simTime().dbl()) > ArpTable[i][2]){    //mean life time ended
                for(int j=0;j<3;j++){                               //erase the line
                        ArpTable[i][j] = "";
                }
            }
        }
    }
}


void ethernet::sendMsg(cMessage* msg){

    cChannel *txchannel = gate("switch$o")->getTransmissionChannel();
    simtime_t txFinishTime = txchannel->getTransmissionFinishTime();
    if(txFinishTime >= simTime()){
         txFinishTime = txFinishTime-simTime();
         sendDelayed(msg,txFinishTime,"switch$o");
    }
    else{
        send(msg,"switch$o");
    }
}





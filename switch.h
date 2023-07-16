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

#ifndef __LAB_2_FINAL_SWITCH_H_
#define __LAB_2_FINAL_SWITCH_H_
#define num_of_host 4

#include "Eth_pck_m.h"
#include "IP_pck_m.h"
#include "App_pck_m.h"
#include "ARP_m.h"
#include <omnetpp.h>

using namespace omnetpp;

/**
 * TODO - Generated class
 */

struct DataBaseNode
{
    std::string SourceMac;
    double agingtime;
    int gate;
    DataBaseNode *next;
} DataBaseNode;

class Switch : public cSimpleModule
{
  private:
    //std::string DataBase[num_of_host][3];  //[mac][gate][ttl]
    double life_time;
    double sLatency;
    double delay_send;
    int gateOut;
    int arrivalGate;
    struct DataBaseNode *DataBaseHead = NULL;

  public:
    int roee;




  protected:
    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    virtual void updateDataBase();
    virtual void handleEthMsg(cMessage *msg);
    virtual int search(std::string s);
    virtual void append(std::string mac, int gate, double aging);
    virtual void broadcastEth(int g, Eth_pck* copy);
};

#endif

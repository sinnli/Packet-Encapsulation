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

#include <stdio.h>
#include <string.h>
#include <omnetpp.h>
using namespace omnetpp;
#include "Application.h"
#include "App_pck_m.h"
#include "IP_pck_m.h"
#include <math.h>


Define_Module(Application);

void Application::initialize()
{
    curr = simTime();
    prev = simTime();
    numReceived = 0;
    event = new cMessage("event");
    delayVector.setName("delay vec");
    intervalVector.setName("interval vec");


    //raflle interval to new app message
    simtime_t timeDelay = par("delayTime");
    scheduleAt(simTime() + timeDelay, event);

}

void Application::handleMessage(cMessage *msg)
{
    //self messsage
    if(msg->isSelfMessage()){

        //generate random msg length
        int size;
        do{
            size  = par("pck_len");
        }while(size<26 || size>1480);

        while(size%4 != 0){
           size++;
        }

        //generate and send app new messag
        App_pck *app_msg = new App_pck();
        app_msg->setKind(3);
        app_msg->setByteLength(size);
        send(app_msg, "ip$o");

        //raflle interval to new app message
        simtime_t timeDelay = par("delayTime");
        scheduleAt(simTime() + timeDelay, event);
    }

    //app message
    else if(msg->getKind() == 3){
        App_pck *app_msg = check_and_cast<App_pck *>(msg);

        //calculate stats
        numReceived++;

        curr = simTime();
        interval = curr - prev;
        intervalVector.record(interval);
        intervalStats.collect(interval);

        delay = curr - app_msg->getCreationTime();
        printf("%s delay is %f\n",getFullPath().c_str(),delay.dbl());
        delayVector.record(delay.dbl());
        delayStats.collect(delay.dbl());

        prev = simTime();

    }
}

void Application::finish(){

    EV << "------------------------------------------------------------------" << endl;
    EV << getFullPath() << " Statistics: "<< endl;
    EV << "------------------------------------------------------------------" << endl;

    EV << "Total APP Packets Received: " << numReceived << endl;

    EV << "------------------------------------------------------------------" << endl;

    EV << "Arrival Intervals Statistics:"<< endl;

    EV << "arrival intervals, Min: " << intervalStats.getMin() << " sec" << endl;
    EV << "arrival intervals, Max: " << intervalStats.getMax() << " sec" << endl;
    EV << "arrival intervals, Mean: " << intervalStats.getMean() << " sec" << endl;
   // EV << "arrival intervals, StdDev: " << intervalStats.getStddev() << " sec" << endl;
    EV << "arrival intervals, Variance: " << intervalStats.getVariance() << " sec" << endl;

    EV << "------------------------------------------------------------------" << endl;
    EV << "Delays Statistics:"<< endl;

    EV << "delay, Min: " << delayStats.getMin() << " sec" << endl;
    EV << "delay, Max: " << delayStats.getMax() << " sec" << endl;
    EV << "delay, Mean: " << delayStats.getMean() << " sec" << endl;
   // EV << "delay, StdDev: " << delayStats.getStddev() << " sec" << endl;
    EV << "delay, Variance: " << delayStats.getVariance() << " sec" << endl;

    EV << "------------------------------------------------------------------" << endl;


    intervalStats.recordAs("arrival intervals");
    delayStats.recordAs("delays");

}

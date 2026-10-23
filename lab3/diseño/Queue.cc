#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>
#include "FeedbackPkt_m.h"


using namespace omnetpp;

class Queue: public cSimpleModule {
private:
    cQueue buffer;
    cMessage *endServiceEvent;
    cOutVector bufferSizeVector;
    cOutVector packetDropVector;
    int packetsDropped;
    int bufferMaxSize;
    int limit;
    int base;
    int balance;
    bool positiveAlerts;
    void SendFeedback(double damage, bool status);
public:
    Queue();
    virtual ~Queue();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Queue);

Queue::Queue() {
    endServiceEvent = NULL;
}

Queue::~Queue() {
    cancelAndDelete(endServiceEvent);
}

void Queue::initialize() {
    buffer.setName("buffer");
    bufferSizeVector.setName("Paquetes En Buffer");
    packetDropVector.setName("Paquetes Eliminados");
    endServiceEvent = new cMessage("endService");
    packetsDropped = 0;
    bufferMaxSize = par("bufferSize").intValue();
    limit = 0.70 * bufferMaxSize;
    base = 0.30 * bufferMaxSize;
    positiveAlerts = false;
    balance = 0;
}

void Queue::finish() {
    recordScalar("Dropped packets", packetsDropped);
}

void Queue::SendFeedback(double damage, bool status) {
    FeedbackPkt *feedbackPkt = new FeedbackPkt();
    feedbackPkt->setKind(2);
    feedbackPkt->setByteLength(20);
    feedbackPkt->setFullBuffer(status);
    feedbackPkt->setLiberateBuffer(!status);
    feedbackPkt->setDamageLevel(damage);
    buffer.insertBefore(buffer.front(), feedbackPkt);
}

void Queue::handleMessage(cMessage *msg) {

    // if msg is signaling an endServiceEvent
    if (msg == endServiceEvent) {
        // if packet in buffer, send next one
        if (!buffer.isEmpty()) {
            // dequeue packet
            cPacket *pkt = (cPacket*) buffer.pop();
            bufferSizeVector.record(buffer.getLength());
            // send packet
            send(pkt, "out");
            // start new service
            simtime_t serviceTime = pkt->getDuration();
            scheduleAt(simTime() + serviceTime, endServiceEvent);
        }
    } else { // if msg is a data packet

        // Bubble informativa del tamaño de la Queue
        char str[100];
        gcvt(buffer.getLength(), 10, str);
        bubble(str);

        // check buffer limit
        if (buffer.getLength() >= bufferMaxSize) {
            delete msg;
            this->bubble("packet dropped");
            packetsDropped++;
            packetDropVector.record(packetsDropped);
        } else {
            char str[100];
            sprintf(str, "%d", buffer.getLength());
            bubble(str);

            if(buffer.getLength() >= limit){ 
                SendFeedback(5.0, true);
                positiveAlerts = true;
                balance += 5;
            }
            if(positiveAlerts && balance>=5 && buffer.getLength() <= base){ 
                SendFeedback(5.0, false);
                balance -= 5;
            }

            // enqueue the packet
            buffer.insert(msg);
            bufferSizeVector.record(buffer.getLength());
            // if the server is idle
            if (!endServiceEvent->isScheduled()) {
                // start the service
                scheduleAt(simTime(), endServiceEvent);
            }
        }
    }
}

#endif /* QUEUE */

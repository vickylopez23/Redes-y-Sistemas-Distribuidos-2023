#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>
#include "FeedbackPkt_m.h"

using namespace omnetpp;

class TransportTx: public cSimpleModule {
private:
    cQueue buffer;
    cOutVector bufferSizeVector;
    cMessage *endServiceEvent;
    double magicNumber;

    cOutVector packetDropVector;
    int packetsDropped;

public:
    TransportTx();
    virtual ~TransportTx();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
    virtual void fixCongestion(double x);
    virtual void liberateCongestion(double x);
};

Define_Module(TransportTx);

TransportTx::TransportTx() {
    endServiceEvent = NULL;
}

TransportTx::~TransportTx() {
    cancelAndDelete(endServiceEvent);
}

void TransportTx::initialize() {
    buffer.setName("buffer");
    bufferSizeVector.setName("Paquetes En Buffer");
    packetDropVector.setName("Paquetes dropeados");
    packetsDropped = 0;
    magicNumber = 0.0;
    endServiceEvent = new cMessage("endService");
}

void TransportTx::finish() {
    recordScalar("Dropped packets", packetsDropped);
}

void TransportTx::fixCongestion(double x) {
    magicNumber = magicNumber + (x / 100);
}
;
void TransportTx::liberateCongestion(double x) {
    magicNumber = magicNumber - (x / 100);
}

void TransportTx::handleMessage(cMessage *msg) {
    // check msg type
    if (msg->getKind() == 2) { // Si es un mensaje de control / feedback
        FeedbackPkt *feedbackPkt = (FeedbackPkt*) msg;
        double levelDamage = feedbackPkt->getDamageLevel();
        if (feedbackPkt->getFullBuffer()) {
            // hacer algo con esto TODO
            fixCongestion(levelDamage);
        } else if (feedbackPkt->getLiberateBuffer()) {
            liberateCongestion(levelDamage);
        }
        delete (msg);
    } else {
        // if msg is signaling an endServiceEvent
        if (msg == endServiceEvent) {
            // if packet in buffer, send next one
            if (!buffer.isEmpty()) {
                // dequeue packet
                this->bubble("endServiceEvent");
                cPacket *pkt = (cPacket*) buffer.pop();
                bufferSizeVector.record(buffer.getLength());
                // send packet
                send(pkt, "toOut$o");
                // start new service
                simtime_t serviceTime = pkt->getDuration();
                // toDelete
                // simtime_t serviceTime = pkt->getDuration();
                serviceTime = (serviceTime + (serviceTime * magicNumber));
                scheduleAt(simTime() + serviceTime, endServiceEvent);
            }
        } else { // if msg is a data packet
            // check buffer limit
            if (buffer.getLength() >= par("bufferSize").intValue()) {
                delete msg;
                this->bubble("Packet Dropped");
                packetsDropped++;
                packetDropVector.record(packetsDropped);
            } else {
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
}

#endif /* TRANSPORTTX */

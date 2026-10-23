#ifndef QUEUE
#define QUEUE

#include <string.h>
#include <omnetpp.h>
#include "FeedbackPkt_m.h"

using namespace omnetpp;

class TransportRx: public cSimpleModule {
private:
    cQueue buffer;
    cOutVector bufferSizeVector;
    cQueue bufferFeedback;
    cMessage *endServiceEvent;
    cMessage *feedbackEvent;
    void SendFeedback(double damage, bool status);

    int magicNumber;
    int balance;
    bool positiveAlerts;
    void Enqueue(cMessage *msg);
    
    int bufferMaxSize;
    int limit1;
    int limit2;
    int limit3;
    int base1;
    int base2;
    int base3;
    
    cOutVector packetDropVector;
    int packetsDropped;
public:
    TransportRx();
    virtual ~TransportRx();

protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(TransportRx);

TransportRx::TransportRx() {
    endServiceEvent = NULL;
    feedbackEvent = NULL;
}

TransportRx::~TransportRx() {
    cancelAndDelete(endServiceEvent);
    cancelAndDelete(feedbackEvent);
}

void TransportRx::initialize() {
    buffer.setName("buffer");
    bufferSizeVector.setName("Paquetes En Buffer");
    bufferFeedback.setName("bufferFeedback");
    magicNumber = 0.0;
    balance = 0;
    positiveAlerts = false;

    endServiceEvent = new cMessage("endService");
    feedbackEvent = new cMessage("feedbackEvent");

    // check buffer limit
    bufferMaxSize = par("bufferSize").intValue();
    limit1 = 0.90 * bufferMaxSize;
    limit2 = 0.80 * bufferMaxSize;
    limit3 = 0.70 * bufferMaxSize;
    base1 = 0.30 * bufferMaxSize;
    base2 = 0.20 * bufferMaxSize;
    base3 = 0.10 * bufferMaxSize;

    packetDropVector.setName("Paquetes dropeados");
    packetsDropped = 0;
}

void TransportRx::finish() {
    recordScalar("Dropped packets", packetsDropped);
}

void TransportRx::Enqueue(cMessage *msg) {
    if (buffer.getLength() >= bufferMaxSize) {
        delete msg;
        this->bubble("pkt drop");
        packetsDropped++;
        packetDropVector.record(packetsDropped);
        //TODO juntar stats paquetes eliminados
    } else {
        // podriamos hacerlo mas lindo sin harcodear dividiendo lo que tengo por
        // el maximo y asi consiguiendo un numero de damage variable
        if (buffer.getLength() >= limit3) {
            double damage = 5.0;
            if (buffer.getLength() >= limit2) {
                damage = 10.0;
            } else if (buffer.getLength() >= limit1) {
                damage = 15.0;
            }
            SendFeedback(damage, true);
            positiveAlerts = true;
            balance += damage;
        }

        // alertas positivas para mandar mas rapido
        if (positiveAlerts && balance >= 5.0 && buffer.getLength() <= base1) {
            double damage = 5.0;
            if (positiveAlerts && balance >= 10.0 && buffer.getLength() <= base2) {
                damage = 10.0;
            } else if (positiveAlerts && balance >= 15.0 && buffer.getLength() <= base3) {
                damage = 15.0;
            }
            SendFeedback(damage, false);
            balance -= damage;
            positiveAlerts = (balance > 0);
        }

        buffer.insert(msg);
        bufferSizeVector.record(buffer.getLength());
        if (!endServiceEvent->isScheduled()) {
            scheduleAt(simTime(), endServiceEvent);
        }
    }
}

// genera un pkt feedback y lo encola en el buffer de feedbakcs, tmb lanza un feedbackevent
void TransportRx::SendFeedback(double damage, bool status) {
    FeedbackPkt *feedbackPkt = new FeedbackPkt();
    feedbackPkt->setKind(2);
    feedbackPkt->setByteLength(20);
    feedbackPkt->setFullBuffer(status);
    feedbackPkt->setLiberateBuffer(!status);
    feedbackPkt->setDamageLevel(damage);

    bufferFeedback.insert(feedbackPkt);
    // ver si ya estamos enviando otro
    if (!feedbackEvent->isScheduled()) {
        scheduleAt(simTime() + 0, feedbackEvent);
    }
}

void TransportRx::handleMessage(cMessage *msg) {
    char str[100];
    sprintf(str, "%d", balance);
    bubble(str);
    // check msg type
    if (msg->getKind() == 2) {
        FeedbackPkt *feedbackPkt = check_and_cast<FeedbackPkt *>(msg);
        bufferFeedback.insert(feedbackPkt);
        if (!feedbackEvent->isScheduled()) {
            scheduleAt(simTime() + 0, feedbackEvent);
        }
    } else if (msg->getKind() == 0) {
        // si el mensaje es una de nuestras señales
        if (msg == endServiceEvent) {
            if (!buffer.isEmpty()) {
                cPacket *pkt = (cPacket*) buffer.pop();
                bufferSizeVector.record(buffer.getLength());
                send(pkt, "toApp");
                simtime_t serviceTime = pkt->getDuration();
                scheduleAt(simTime() + serviceTime, endServiceEvent);
            }
        } else if (msg == feedbackEvent) {
            if (!bufferFeedback.isEmpty()) {
                cPacket *pkt = (cPacket*) bufferFeedback.pop();
                send(pkt, "toOut$o");
                simtime_t serviceTime = pkt->getDuration();
                scheduleAt(simTime() + serviceTime, feedbackEvent);
            }
        } else { // if msg is a data packet
            Enqueue(msg);
        }
    }
}

#endif /* TRANSPORTRX */

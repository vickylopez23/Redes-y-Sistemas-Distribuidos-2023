#ifndef NET
#define NET

#include <string.h>
#include <omnetpp.h>
#include <packet_m.h>
// #include "ShearMsg_m.h"

#include <iostream>
#include <utility>
using namespace std;

using namespace omnetpp;

int NODE_INFORMATION_PACKET_KIND = 2;

class Net: public cSimpleModule {
private:
    int networkExternalNodes;
    // Vector que representa tabla de envio de paquetes a un nodo X y su coste D por izquierda
    std::vector<pair<int, int>> routingTable;
    void shareNodeInformation();
    void readNodeInformation(Packet *pkt);
    pair<int, pair<int, int>> getRoutingTableEntry(int node);
    void sendPktToOut(Packet *pkt, bool toNextNode);
public:
    Net();
    virtual ~Net();
protected:
    virtual void initialize();
    virtual void finish();
    virtual void handleMessage(cMessage *msg);
};

Define_Module(Net);

#endif /* NET */

Net::Net() {
}

Net::~Net() {
}

void Net::initialize() {
    networkExternalNodes = 0;
    shareNodeInformation();
}

void Net::finish() {
}

pair<int, pair<int, int>> Net::getRoutingTableEntry(int node){
    auto result = std::make_pair(-1, std::make_pair(-1, -1));
    for(int i = 0; i < networkExternalNodes; ++i) {
        if(routingTable[i].first == node) {
            result.first = i;
            auto entry = routingTable[i];
            result.second.first = entry.first;
            result.second.second = entry.second;
            break;
        }
    }
    return result; 
}

void Net::readNodeInformation(Packet *pkt){
    if(pkt->getSource() == this->getParentModule()->getIndex()){
        delete(pkt);
    } else {
        auto node = getRoutingTableEntry(pkt->getSource());
        if(node.first < 0) {
            // Agregamos a la tabla de enrutamiento
            networkExternalNodes += 1;
            if(routingTable.size() <= networkExternalNodes) {
                routingTable.resize(networkExternalNodes * 2);
            }
            auto table_entry = std::make_pair(pkt->getSource(), pkt->getHopCount());
            routingTable[networkExternalNodes - 1].first = table_entry.first;
            routingTable[networkExternalNodes - 1].second = table_entry.second;
            node.first = networkExternalNodes;
            node.second = table_entry;
        } else if(pkt->getHopCount() < node.second.second) {
            routingTable[node.first].first = node.second.first;
            routingTable[node.first].second = pkt->getHopCount();
        }
        sendPktToOut(pkt, true);
    }
}

void Net::sendPktToOut(Packet *pkt, bool toNextNode){
    pkt->setHopCount(pkt->getHopCount()+1);
    if(toNextNode){
        send(pkt, "toLnk$o", 0);
        return;
    }
    pair<int, pair<int, int>> entry = getRoutingTableEntry(pkt->getDestination());
    if(entry.first < 0) {
        error("Paquete tiene destino no registrado en la tabla de enrutamiento.");
        delete pkt;
        return;
    }
    int gate = 1; // Defaults to left gate
    int costToRight = networkExternalNodes - entry.second.second;
    if(entry.second.second > costToRight) // If it is better, send it to the right gate;
        gate = 0;
    send(pkt, "toLnk$o", gate);
}

void Net::handleMessage(cMessage *msg) {

    // All msg (events) on net are packets
    Packet *pkt = (Packet *) msg;
    if(pkt->getKind() == NODE_INFORMATION_PACKET_KIND) {
        readNodeInformation(pkt);
        return;
    }

    // If this node is the final destination, send to App
    if (pkt->getDestination() == this->getParentModule()->getIndex()) {
        send(msg, "toApp$o");
        return;
    }
    // We recieved a packet to send outside.
    sendPktToOut(pkt, false);
}

void Net::shareNodeInformation() {
    Packet *shareInfoPacket = new Packet();
    shareInfoPacket->setKind(NODE_INFORMATION_PACKET_KIND);
    shareInfoPacket->setSource(this->getParentModule()->getIndex()); 
    shareInfoPacket->setHopCount(1);
    send(shareInfoPacket, "toLnk$o", 0);
}

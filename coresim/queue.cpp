#include <iostream>
#include <stdlib.h>
#include "assert.h"

#include "queue.h"
#include "packet.h"
#include "event.h"
#include "debug.h"

#include "../run/params.h"

extern double get_current_time(); // TODOm
extern void add_to_event_queue(Event* ev);
extern uint32_t dead_packets;
extern DCExpParams params;


/* Queues */
Queue::Queue(uint32_t id, double rate, uint32_t limit_bytes, int location) {
    this->id = id;
    this->rate = rate; // in bps
    this->limit_bytes = limit_bytes;
    this->bytes_in_queue = 0;
    this->busy = false;
    this->queue_proc_event = NULL;
    //this->packet_propagation_event = NULL;
    this->location = location;
    this->propagation_delay = params.propagation_delay;
    this->p_arrivals = 0; this->p_departures = 0;
    this->b_arrivals = 0; this->b_departures = 0;

    this->pkt_drop = 0;
    this->spray_counter = std::rand();
    this->packet_transmitting = NULL;
}

void Queue::set_src_dst(Node *src, Node *dst) {
    this->src = src;
    this->dst = dst;
}


void Queue::enque(Packet *packet) {
    p_arrivals += 1;
    b_arrivals += packet->size;
    packets.push_back(packet);
    bytes_in_queue += packet->size;
    if (bytes_in_queue + packet->size > limit_bytes)
    {
        pkt_drop++;
        size_t index = packets.size()-1;
        Packet* p = packets[index];
        packets.erase(packets.begin() + index);
        bytes_in_queue -= p->size;
        drop(p);
        return;
    }
}

Packet *Queue::deque() {
    if (bytes_in_queue > 0) {
        size_t index = 0;
        Packet *p = packets[index];
        packets.erase(packets.begin()+index);
        bytes_in_queue -= p->size;
        p_departures += 1;
        b_departures += p->size;
        return p;
    }
    return NULL;
}

void Queue::drop(Packet *packet) {

    packet->flow->pkt_drop++;
    if(packet->seq_no < packet->flow->size){
        packet->flow->data_pkt_drop++;
    }


    packet->flow->tracker.notifyPacketDrop(packet, this);


    //if (location != 0 && packet->type == NORMAL_PACKET) {
        dead_packets += 1;
    //}

    if(debug_flow(packet->flow->id))
        std::cout << get_current_time() << " pkt drop. flow:" << packet->flow->id
            << " seq:" << packet->seq_no
            << " at queue id:" << this->id << " loc:" << this->location << "\n";
}

double Queue::get_transmission_delay(uint32_t size) {
    return size * 8.0 / rate;
}


HostQueue::HostQueue(uint32_t id, double rate, uint32_t limit_bytes, int location) : Queue(id, rate, limit_bytes, location) {
    flows.clear();
}

void HostQueue::enque(Packet *packet) {
    while (true) {}
}

void HostQueue::drop(Packet *packet) {
    while (true) {}
}

void HostQueue::registerFlow(Flow *flow) {
    flows.push_back(flow);
}

Packet *HostQueue::deque() {
    if (flows.size() == 0)
    {
        return nullptr;
    }
    Flow* first = *flows.begin();
    Packet* p = first->send_pending_data();
    if (first->finished)
    {
        flows.pop_front();
    }
    return p;
}

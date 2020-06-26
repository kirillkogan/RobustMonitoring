#include <math.h>
#include <iostream>
#include <assert.h>
#include <algorithm>

#include "flow.h"
#include "packet.h"
#include "event.h"

#include "../run/params.h"
#include "../run/flow_generator.h"

extern double get_current_time(); 
extern void add_to_event_queue(Event *);
extern int get_event_queue_size();
extern DCExpParams params;
extern uint32_t num_outstanding_packets;
extern uint32_t max_outstanding_packets;
extern uint32_t duplicated_packets_received;

Flow::Flow(uint32_t id, double start_time, uint32_t size_, Host *s, Host *d) : tracker(this, (int)ceil((double)size_/params.mss)) {
    this->id = id;
    this->start_time = start_time;
    this->finish_time = 0;
    this->size = size_;
    this->src = s;
    this->dst = d;

    this->next_seq_no = 0;
    this->last_unacked_seq = 0;

    this->received_bytes = 0;
    this->finished = false;

    //SACK
    this->scoreboard_sack_bytes = 0;

    this->mss = params.mss;
    this->hdr_size = params.hdr_size;
    this->total_pkt_sent = 0;
    this->size_in_pkt = (int)ceil((double)size/mss);

    this->pkt_drop = 0;
    this->data_pkt_drop = 0;
    this->ack_pkt_drop = 0;
    this->flow_priority = 0;
    this->first_byte_send_time = -1;
    this->first_byte_receive_time = -1;
    this->first_hop_departure = 0;
    this->last_hop_departure = 0;
}

Flow::~Flow() {
    //  packets.clear();
}

void Flow::start_flow() {
    HostQueue* queue = (HostQueue*)src->queue;
    if (!queue->busy) {
        queue->queue_proc_event = new QueueProcessingEvent(get_current_time(), queue);
        add_to_event_queue(queue->queue_proc_event);
        queue->busy = true;
        queue->packet_transmitting = nullptr;
    }
    queue->registerFlow(this);
}

Packet* Flow::send_pending_data() {
    Packet *p = send(next_seq_no);
    p->flow->tracker.notifyPacketEnque(p, src->queue);
    next_seq_no = std::min(next_seq_no + mss, size);
    finished |= next_seq_no == size;
    return p;
}

Packet *Flow::send(uint32_t seq) {
    Packet *p = NULL;

    uint32_t pkt_size;
    if (seq + mss > this->size) {
        pkt_size = this->size - seq + hdr_size;
    } else {
        pkt_size = mss + hdr_size;
    }

    p = new Packet(
            this,
            seq, 
            pkt_size,
            src, 
            dst
            );
    this->total_pkt_sent++;

//    add_to_event_queue(new PacketQueuingEvent(get_current_time(), p, src->queue));
    return p;
}



void Flow::receive(Packet *p) {
    received_count++;
    received_bytes += (p->size - hdr_size);
    total_queuing_time += p->total_queuing_delay;

    if(num_outstanding_packets >= ((p->size - hdr_size) / (mss)))
        num_outstanding_packets -= ((p->size - hdr_size) / (mss));
    else
        num_outstanding_packets = 0;

    if (received_bytes == this->size)
    {
        finished = true;
        finish_time = get_current_time();
        flow_completion_time = finish_time - start_time;
        FlowFinishedEvent *ev = new FlowFinishedEvent(get_current_time(), this);
        add_to_event_queue(ev);
    }
    delete p;
}
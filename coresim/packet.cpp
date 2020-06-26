#include <iostream>
#include "packet.h"
#include "../run/params.h"

extern DCExpParams params;

Packet::Packet(
        Flow *flow,
        uint32_t seq_no, 
        uint32_t size,
        Host *src, 
        Host *dst
    ) {
    this->flow = flow;
    this->seq_no = seq_no;
    this->size = size;
    this->src = src;
    this->dst = dst;

    this->total_queuing_delay = 0;
    this->packet_number_by_first_switch = -2;
}

Packet::~Packet() {
}


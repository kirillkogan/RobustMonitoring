#ifndef FLOW_H
#define FLOW_H

#include <unordered_map>
#include <map>
#include "node.h"
#include "LDgammaTracker.h"

class Packet;

class Flow {
    public:
        Flow(uint32_t id, double start_time, uint32_t size, Host *s, Host *d);

        ~Flow(); // Destructor

        virtual void start_flow();
        virtual Packet* send_pending_data();
        virtual Packet *send(uint32_t seq);
        virtual void receive(Packet *p);

        uint32_t id;
        double start_time;
        double finish_time;
        uint32_t size;
        Host *src;
        Host *dst;
        uint32_t mss;
        uint32_t hdr_size;

        // Sender variables
        uint32_t next_seq_no;
        uint32_t last_unacked_seq;

        uint32_t received_bytes;

        uint32_t total_pkt_sent;
        int size_in_pkt;
        int pkt_drop;
        int data_pkt_drop;
        int ack_pkt_drop;
        int first_hop_departure;
        int last_hop_departure;
        uint32_t received_count;
        // Sack
        uint32_t scoreboard_sack_bytes;
        // finished variables
        bool finished;
        double flow_completion_time;
        double total_queuing_time;
        double first_byte_send_time;
        double first_byte_receive_time;

        uint32_t flow_priority;

        LDgammaTracker tracker;
};

#endif

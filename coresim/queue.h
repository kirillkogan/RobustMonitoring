#ifndef QUEUE_H
#define QUEUE_H

#include <deque>
#include <stdint.h>
#include <vector>

#define DROPTAIL_QUEUE 1

class Node;
class Packet;
class Event;

class QueueProcessingEvent;
class PacketPropagationEvent;

class Queue {
    public:
        Queue(uint32_t id, double rate, uint32_t limit_bytes, int location);
        void set_src_dst(Node *src, Node *dst);
        virtual void enque(Packet *packet);
        virtual Packet *deque();
        virtual void drop(Packet *packet);
        double get_transmission_delay(uint32_t size);

        // Members
        uint32_t id;
        double rate;
        uint32_t limit_bytes;
        std::deque<Packet *> packets;
        uint32_t bytes_in_queue;
        bool busy;
        QueueProcessingEvent *queue_proc_event;

        std::vector<Event*> busy_events;
        Packet* packet_transmitting;

        Node *src;
        Node *dst;

        uint64_t b_arrivals, b_departures;
        uint64_t p_arrivals, p_departures;

        double propagation_delay;
        bool interested;

        uint64_t pkt_drop;
        uint64_t spray_counter;

        int location;
};


class Flow;
class HostQueue : public Queue {
public:
    HostQueue(uint32_t id, double rate, uint32_t limit_bytes, int location);
    virtual void enque(Packet *packet);
    virtual void registerFlow(Flow* flow);
    virtual Packet *deque();
    virtual void drop(Packet *packet);

    std::deque<Flow*> flows;
};

#endif

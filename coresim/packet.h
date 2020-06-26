#ifndef PACKET_H
#define PACKET_H

#include "flow.h"
#include "node.h"
#include <stdint.h>
// TODO: Change to Enum
#define NORMAL_PACKET 0

class FastpassEpochSchedule;

class Packet {

    public:
        Packet(Flow *flow, uint32_t seq_no, uint32_t size, Host *src, Host *dst);

        Flow *flow;
        uint32_t seq_no;
        uint32_t size;
        Host *src;
        Host *dst;
        double total_queuing_delay;
        double last_enque_time;


        int packet_number_by_first_switch;

        virtual ~Packet();
};


#endif


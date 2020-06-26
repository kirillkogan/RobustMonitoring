#ifndef TOPOLOGY_H
#define TOPOLOGY_H

#include <cstddef>
#include <iostream>
#include <math.h>
#include <vector>

#include "node.h"
#include "assert.h"
#include "packet.h"
#include "queue.h"

#include "../run/params.h"

class Topology {
    public:
        Topology();
        virtual Queue *get_next_hop(Packet *p, Queue *q) = 0;
        std::vector<Host *> hosts;
        std::vector<Switch*> switches;
};

class LeafSpineTopology : public Topology {
    public:
        LeafSpineTopology(
                uint32_t num_hosts, 
                uint32_t num_agg_switches,
                uint32_t num_core_switches, 
                double bandwidth
                );

        virtual Queue* get_next_hop(Packet *p, Queue *q);


        std::vector<AggSwitch*> agg_switches;
        std::vector<CoreSwitch*> core_switches;
};

#endif

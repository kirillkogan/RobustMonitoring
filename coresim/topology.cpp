#include "topology.h"

extern DCExpParams params;

/*
   uint32_t num_hosts = 144;
   uint32_t num_agg_switches = 9;
   uint32_t num_core_switches = 4;
   */
Topology::Topology() {}

/*
 * Leaf-spine topology with 144 hosts (16, 9, 4)
 */
LeafSpineTopology::LeafSpineTopology(
        uint32_t num_hosts, 
        uint32_t num_agg_switches,
        uint32_t num_core_switches, 
        double bandwidth
        ) : Topology () {
    uint32_t hosts_per_agg_switch = num_hosts / num_agg_switches;

    //Capacities
    double c1 = bandwidth;
    double c2 = hosts_per_agg_switch * bandwidth / num_core_switches;

    // Create Hosts
    for (uint32_t i = 0; i < num_hosts; i++) {
        hosts.push_back(new Host(i, c1));
    }

    // Create Switches
    for (uint32_t i = 0; i < num_agg_switches; i++) {
        AggSwitch* sw = new AggSwitch(i, hosts_per_agg_switch, c1, num_core_switches, c2);
        agg_switches.push_back(sw); // TODO make generic
        switches.push_back(sw);
    }
    for (uint32_t i = 0; i < num_core_switches; i++) {
        CoreSwitch* sw = new CoreSwitch(i + num_agg_switches, num_agg_switches, c2);
        core_switches.push_back(sw);
        switches.push_back(sw);
    }

    //Connect host queues
    for (uint32_t i = 0; i < num_hosts; i++) {
        hosts[i]->queue->set_src_dst(hosts[i], agg_switches[i/16]);
        //std::cout << "Linking Host " << i << " to Agg " << i/16 << "\n";
    }

    // For agg switches -- REMAINING
    for (uint32_t i = 0; i < num_agg_switches; i++) {
        // Queues to Hosts
        for (uint32_t j = 0; j < hosts_per_agg_switch; j++) { // TODO make generic
            Queue *q = agg_switches[i]->queues[j];
            q->set_src_dst(agg_switches[i], hosts[i * 16 + j]);
            //std::cout << "Linking Agg " << i << " to Host" << i * 16 + j << "\n";
        }
        // Queues to Core
        for (uint32_t j = 0; j < num_core_switches; j++) {
            Queue *q = agg_switches[i]->queues[j + 16];
            q->set_src_dst(agg_switches[i], core_switches[j]);
            //std::cout << "Linking Agg " << i << " to Core" << j << "\n";
        }
    }

    //For core switches -- PERFECT
    for (uint32_t i = 0; i < num_core_switches; i++) {
        for (uint32_t j = 0; j < num_agg_switches; j++) {
            Queue *q = core_switches[i]->queues[j];
            q->set_src_dst(core_switches[i], agg_switches[j]);
            //std::cout << "Linking Core " << i << " to Agg" << j << "\n";
        }
    }
}


Queue *LeafSpineTopology::get_next_hop(Packet *p, Queue *q) {
    if (q->dst->type == HOST) {
        return NULL; // Packet Arrival
    }

    // At host level
    if (q->src->type == HOST) { // Same Rack or not
        assert (p->src->id == q->src->id);

        if (p->src->id / 16 == p->dst->id / 16) {
            return ((Switch *) q->dst)->queues[p->dst->id % 16];
        } 
        else {
            uint32_t hash_port = 0;
            if(params.load_balancing == 0)
            {
                hash_port = (q->spray_counter / params.flowlet_size)%4;
                q->spray_counter++;
            }
            else if(params.load_balancing == 1)
                hash_port = (p->src->id + p->dst->id + p->flow->id) % 4;
            return ((Switch *) q->dst)->queues[16 + hash_port];
        }
    }

    // At switch level
    if (q->src->type == SWITCH) {
        if (((Switch *) q->src)->switch_type == AGG_SWITCH) {
            return ((Switch *) q->dst)->queues[p->dst->id / 16];
        }
        if (((Switch *) q->src)->switch_type == CORE_SWITCH) {
            return ((Switch *) q->dst)->queues[p->dst->id % 16];
        }
    }

    assert(false);
}




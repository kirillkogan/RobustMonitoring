#include <iomanip>

#include "event.h"
#include "packet.h"
#include "topology.h"
#include "debug.h"

#include "../run/params.h"

extern Topology* topology;
extern std::priority_queue<Event*, std::vector<Event*>, EventComparator> event_queue;
extern double current_time;
extern DCExpParams params;
extern std::deque<Event*> flow_arrivals;
extern std::deque<Flow*> flows_to_schedule;

extern uint32_t num_outstanding_packets;
extern uint32_t max_outstanding_packets;

extern uint32_t num_outstanding_packets_at_50;
extern uint32_t num_outstanding_packets_at_100;
extern uint32_t arrival_packets_at_50;
extern uint32_t arrival_packets_at_100;
extern uint32_t arrival_packets_count;
extern uint32_t total_finished_flows;

extern uint32_t backlog3;
extern uint32_t backlog4;
extern uint32_t duplicated_packets_received;
extern uint32_t duplicated_packets;
extern uint32_t injected_packets;
extern uint32_t completed_packets;
extern uint32_t total_completed_packets;
extern uint32_t dead_packets;
extern uint32_t sent_packets;

extern EmpiricalRandomVariable *nv_bytes;

extern double get_current_time();
extern void add_to_event_queue(Event *);
extern int get_event_queue_size();

uint32_t Event::instance_count = 0;

Event::Event(uint32_t type, double time) {
    this->type = type;
    this->time = time;
    this->cancelled = false;
    this->unique_id = Event::instance_count++;
}

Event::~Event() {
}


/* Flow Arrival */
FlowCreationForInitializationEvent::FlowCreationForInitializationEvent(
        double time, 
        Host *src, 
        Host *dst,
        EmpiricalRandomVariable *nv_bytes, 
        RandomVariable *nv_intarr
    ) : Event(FLOW_CREATION_EVENT, time) {
    this->src = src;
    this->dst = dst;
    this->nv_bytes = nv_bytes;
    this->nv_intarr = nv_intarr;
}

FlowCreationForInitializationEvent::~FlowCreationForInitializationEvent() {}

void FlowCreationForInitializationEvent::process_event() {
    uint32_t nvVal, size;
    uint32_t id = flows_to_schedule.size();
    nvVal = (nv_bytes->value() + 0.5); // truncate(val + 0.5) equivalent to round to nearest int
    if (nvVal > 2500000) {
        std::cout << "Giant Flow! event.cpp::FlowCreation:" << 1000000.0 * time << " Generating new flow " << id << " of size " << (nvVal*1460) << " between " << src->id << " " << dst->id << "\n";
        nvVal = 2500000;
    }
    size = (uint32_t) nvVal * 1460;

    if (size != 0) {
        flows_to_schedule.push_back(new Flow(id, time, size, src, dst));
    }

    double tnext = time + nv_intarr->value();
//        std::cout << "event.cpp::FlowCreation:" << 1000000.0 * time << " Generating new flow " << id << " of size "
//         << size << " between " << src->id << " " << dst->id << " " << (tnext - get_current_time())*1e6 << "\n";

    add_to_event_queue(
            new FlowCreationForInitializationEvent(
                tnext,
                src, 
                dst,
                nv_bytes, 
                nv_intarr
                )
            );
}


/* Flow Arrival */
int flow_arrival_count = 0;

FlowArrivalEvent::FlowArrivalEvent(double time, Flow* flow) : Event(FLOW_ARRIVAL, time) {
    this->flow = flow;
}

FlowArrivalEvent::~FlowArrivalEvent() {
}

void FlowArrivalEvent::process_event() {
    //Flows start at line rate; so schedule a packet to be transmitted
    //First packet scheduled to be queued

    num_outstanding_packets += (this->flow->size / this->flow->mss);
    arrival_packets_count += this->flow->size_in_pkt;
    if (num_outstanding_packets > max_outstanding_packets) {
        max_outstanding_packets = num_outstanding_packets;
    }
    this->flow->start_flow();
    flow_arrival_count++;
    if (flow_arrivals.size() > 0) {
        add_to_event_queue(flow_arrivals.front());
        flow_arrivals.pop_front();
    }

    if(params.num_flows_to_run > 10 && flow_arrival_count % 100000 == 0){
        double curr_time = get_current_time();
        uint32_t num_unfinished_flows = 0;
        for (uint32_t i = 0; i < flows_to_schedule.size(); i++) {
            Flow *f = flows_to_schedule[i];
            if (f->start_time < curr_time) {
                if (!f->finished) {
                    num_unfinished_flows ++;
                }
            }
        }
        if(flow_arrival_count == (int)(params.num_flows_to_run * 0.5))
        {
            arrival_packets_at_50 = arrival_packets_count;
            num_outstanding_packets_at_50 = num_outstanding_packets;
        }
        if(flow_arrival_count == params.num_flows_to_run)
        {
            arrival_packets_at_100 = arrival_packets_count;
            num_outstanding_packets_at_100 = num_outstanding_packets;
        }
        std::cerr << "## " << current_time << " NumPacketOutstanding " << num_outstanding_packets
            << " NumUnfinishedFlows " << num_unfinished_flows << " StartedFlows " << flow_arrival_count
            << " StartedPkts " << arrival_packets_count << "\n";
    }
}


/* Packet Queuing */
PacketQueuingEvent::PacketQueuingEvent(double time, Packet *packet,
        Queue *queue) : Event(PACKET_QUEUING, time) {
    this->packet = packet;
    this->queue = queue;
}

PacketQueuingEvent::~PacketQueuingEvent() {
}

void PacketQueuingEvent::process_event() {
    if (!queue->busy) {
        queue->queue_proc_event = new QueueProcessingEvent(get_current_time(), queue);
        add_to_event_queue(queue->queue_proc_event);
        queue->busy = true;
        queue->packet_transmitting = packet;
    }
    queue->enque(packet);
}

/* Packet Arrival */
PacketArrivalEvent::PacketArrivalEvent(double time, Packet *packet)
    : Event(PACKET_ARRIVAL, time) {
        this->packet = packet;
    }

PacketArrivalEvent::~PacketArrivalEvent() {
}

void PacketArrivalEvent::process_event() {
    completed_packets++;
    packet->flow->tracker.notifyPacketEnque(packet, nullptr);
    packet->flow->receive(packet);
}


/* Queue Processing */
QueueProcessingEvent::QueueProcessingEvent(double time, Queue *queue)
    : Event(QUEUE_PROCESSING, time) {
        this->queue = queue;
}

QueueProcessingEvent::~QueueProcessingEvent() {
    if (queue->queue_proc_event == this) {
        queue->queue_proc_event = NULL;
        queue->busy = false; //TODO is this ok??
    }
}

void QueueProcessingEvent::process_event() {
    Packet *packet = queue->deque();
    if (packet != nullptr)
    {
        //std::cerr << "hi "<<"\n";
        // queue->get_transmission_delay(packet->size);
    }
//    std::cerr << "hi\n";
    if (packet) {
        queue->busy = true;
        queue->busy_events.clear();
        queue->packet_transmitting = packet;
        Queue *next_hop = topology->get_next_hop(packet, queue);
 //       std::cerr << next_hop->src->type << " " << HOST << "\n";
        double td = queue->get_transmission_delay(packet->size);
        double pd = queue->propagation_delay;
        double diff = 0;
        if (queue->src->type == HOST && rand() % 4 == 0)
        {
            diff += params.speed*rand()/RAND_MAX*td;
        }

        //double additional_delay = 1e-10;
        queue->queue_proc_event = new QueueProcessingEvent(time + td + diff, queue);
        add_to_event_queue(queue->queue_proc_event);
        queue->busy_events.push_back(queue->queue_proc_event);
        if (next_hop == NULL) {
            Event* arrival_evt = new PacketArrivalEvent(time + td + pd, packet);
            add_to_event_queue(arrival_evt);
            queue->busy_events.push_back(arrival_evt);
        } else {
            Event* queuing_evt = NULL;
            if (params.cut_through == 1) {
                double cut_through_delay =
                    queue->get_transmission_delay(packet->flow->hdr_size);
                queuing_evt = new PacketQueuingEvent(time + cut_through_delay + pd, packet, next_hop);
            } else {
                queuing_evt = new PacketQueuingEvent(time + td + pd, packet, next_hop);
            }
            //std::cerr << ((PacketQueuingEvent*)queuing_evt)->queue->src->type << " aaa\n";
            add_to_event_queue(queuing_evt);
            queue->busy_events.push_back(queuing_evt);
        }
    } else {
        queue->busy = false;
        queue->busy_events.clear();
        queue->packet_transmitting = NULL;
        queue->queue_proc_event = NULL;
    }
}


LoggingEvent::LoggingEvent(double time) : Event(LOGGING, time){
    this->ttl = 1e10;
}

LoggingEvent::LoggingEvent(double time, double ttl) : Event(LOGGING, time){
    this->ttl = ttl;
}

LoggingEvent::~LoggingEvent() {
}

void LoggingEvent::process_event() {
    double current_time = get_current_time();
    // can log simulator statistics here.
}


/* Flow Finished */
FlowFinishedEvent::FlowFinishedEvent(double time, Flow *flow)
    : Event(FLOW_FINISHED, time) {
        this->flow = flow;
    }

FlowFinishedEvent::~FlowFinishedEvent() {}

static int cntF = 0;


void FlowFinishedEvent::process_event() {
    this->flow->finished = true;
    this->flow->finish_time = get_current_time();
    this->flow->flow_completion_time = this->flow->finish_time - this->flow->start_time;
    total_finished_flows++;
    flow->tracker.finalizeResults();
    std::cerr << cntF++ <<"\n";
}

/* Flow Finished */
FlowSendPendingData::FlowSendPendingData(double time, Flow *flow)
        : Event(FLOW_SEND_PENDING_DATA, time) {
    this->flow = flow;
}

FlowSendPendingData::~FlowSendPendingData() {}

void FlowSendPendingData::process_event() {
    this->flow->send_pending_data();
}

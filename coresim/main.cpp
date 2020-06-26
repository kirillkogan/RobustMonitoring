#include <iostream>
#include <algorithm>
#include <fstream>
#include <stdlib.h>
#include <deque>
#include <stdint.h>
#include <time.h>
#include "assert.h"

#include "flow.h"
#include "packet.h"
#include "node.h"
#include "event.h"
#include "topology.h"
#include "queue.h"
#include "random_variable.h"

#include "../run/params.h"

using namespace std;

Topology* topology;
double current_time = 0;
std::priority_queue<Event*, std::vector<Event*>, EventComparator> event_queue;
std::deque<Flow*> flows_to_schedule;
std::deque<Event*> flow_arrivals;

uint32_t num_outstanding_packets = 0;
uint32_t max_outstanding_packets = 0;
uint32_t num_outstanding_packets_at_50 = 0;
uint32_t num_outstanding_packets_at_100 = 0;
uint32_t arrival_packets_at_50 = 0;
uint32_t arrival_packets_at_100 = 0;
uint32_t arrival_packets_count = 0;
uint32_t total_finished_flows = 0;
uint32_t duplicated_packets_received = 0;

uint32_t injected_packets = 0;
uint32_t duplicated_packets = 0;
uint32_t dead_packets = 0;
uint32_t completed_packets = 0;
uint32_t backlog3 = 0;
uint32_t backlog4 = 0;
uint32_t total_completed_packets = 0;
uint32_t sent_packets = 0;

extern DCExpParams params;
double start_time = -1;

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

void add_to_event_queue(Event* ev) {
    event_queue.push(ev);
}

int get_event_queue_size() {
    return event_queue.size();
}

double get_current_time() {
    return current_time; // in us
}



/* Runs a initialized scenario */
void run_scenario() {
    // Flow Arrivals create new flow arrivals
    // Add the first flow arrival
    if (flow_arrivals.size() > 0) {
        add_to_event_queue(flow_arrivals.front());
        flow_arrivals.pop_front();
    }
    int last_evt_type = -1;
    int same_evt_count = 0;
    while (event_queue.size() > 0) {
        Event *ev = event_queue.top();
        event_queue.pop();
        current_time = ev->time;
        if (start_time < 0) {
            start_time = current_time;
        }
        if (ev->cancelled) {
            delete ev; //TODO: Smarter
            continue;
        }
        ev->process_event();

        if(last_evt_type == ev->type && last_evt_type != 9)
            same_evt_count++;
        else
            same_evt_count = 0;

        last_evt_type = ev->type;
        
        if(same_evt_count > 100000){
            std::cout << "Ended event dead loop. Type:" << last_evt_type << "\n";
            break;
        }

        delete ev;
    }

    printf("######## Algorithm 1 statistics ######\n");
    printf("\n");
    printf("E1' = No. of errors outside the interval [|f| - X, 1.02 * |f|] \n");
    printf("\n");


    for (int i = 0; i < 6; i++) {
        printf("Z1(%d, %d) = %.6f, E1(%d, %d) = %.6f,  E1'(%d, %d) = %.6f\n",
                LDgammaTracker::Ns[i], LDgammaTracker::Ts[i], 1.0 * basicAlgorithmStatistics[i][0] / basicAlgorithmStatistics[i][3],
                LDgammaTracker::Ns[i], LDgammaTracker::Ts[i], 1.0 * basicAlgorithmStatistics[i][1] / basicAlgorithmStatistics[i][3],
                LDgammaTracker::Ns[i], LDgammaTracker::Ts[i], 1.0 * basicAlgorithmStatistics[i][2] / basicAlgorithmStatistics[i][3]);
    }


    printf("\n");
    printf("######## Algorithm 2 statistics ######\n");
    printf("\n");

    int globID = 0;
    for (int i = 0; i < 6; i++) {
        for (int gamma = 1; gamma <= ((1 << LDgammaTracker::Ts[i]) - 1); gamma++) {
            printf("Z2(%d, %d) = %.6f, E2(%d, %d) = %.6f,  E2'(%d, %d) = %.6f (gamma = %d)\n",
                   LDgammaTracker::Ns[i], LDgammaTracker::Ts[i],
                   1.0 * algorithm2Statistics[globID][0] / algorithm2Statistics[globID][3],
                   LDgammaTracker::Ns[i], LDgammaTracker::Ts[i],
                   1.0 * algorithm2Statistics[globID][1] / algorithm2Statistics[globID][3],
                   LDgammaTracker::Ns[i], LDgammaTracker::Ts[i],
                   1.0 * algorithm2Statistics[globID][2] / algorithm2Statistics[globID][3],
                   gamma);
            globID++;
        }
    }



    printf("\n");
    printf("######## Real time packet loss ######\n");
    printf("\n");
    printf("λ', λ  are measurement lags for algorithm 4 and algorithm 4* respectively\n");
    printf("\n");


    for (int i = 0; i < 15; i++) {
        printf("G = %d: \n", 5+i*2);
        for (int j = 0; j < 4; j++) {
            printf("Z4(%d, %d) = %.6f, E4(%d, %d) = %.6f,  λ(%d, %d) = %.2f, λ'(%d, %d) = %.2f\n",
                   LDgammaTracker::telemetry_t[j], LDgammaTracker::telemetry_t[j], 1.0 * telemetryStatistics[i][7 * j + 4] / telemetryStatistics[i][7 * j + 6],
                   LDgammaTracker::telemetry_t[j], LDgammaTracker::telemetry_t[j], 1.0 * telemetryStatistics[i][7 * j + 5] / telemetryStatistics[i][7 * j + 6],
                   LDgammaTracker::telemetry_t[j], LDgammaTracker::telemetry_t[j], 1.0 * telemetryStatistics[i][7 * j + 1] / telemetryStatistics[i][7 * j],
                   LDgammaTracker::telemetry_t[j], LDgammaTracker::telemetry_t[j], 1.0 * telemetryStatistics[i][7 * j + 2] / telemetryStatistics[i][7 * j]);
        }
    }


}

extern void run_experiment(int argc, char** argv, uint32_t exp_type);

int main (int argc, char ** argv) {
    time_t start_time;
    time(&start_time);

    //srand(time(NULL));
    srand(0);
    std::cout.precision(15);

    uint32_t exp_type = atoi(argv[1]);
    switch (exp_type) {
        case GEN_ONLY:
        case DEFAULT_EXP:
            run_experiment(argc, argv, exp_type);
            break;
        default:
            assert(false);
    }

    time_t end_time;
    time(&end_time);
}


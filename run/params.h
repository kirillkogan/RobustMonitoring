#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include <fstream>

class DCExpParams {
    public:
        std::string param_str;

        uint32_t initial_cwnd;
        uint32_t max_cwnd;
        double retx_timeout_value;
        uint32_t mss;
        uint32_t hdr_size;
        uint32_t queue_size;
        uint32_t load_balancing; //0 per pkt, 1 per flow

        double propagation_delay;
        double bandwidth;

        uint32_t num_flows_to_run;
        std::string cdf_or_flow_trace;
        uint32_t cut_through;
        uint32_t mean_flow_size;


        uint32_t num_hosts;
        uint32_t num_agg_switches;
        uint32_t num_core_switches;
        double load;
        uint32_t smooth_cdf;
        int flowlet_size;
        double speed;


};

void read_experiment_parameters(std::string conf_filename, uint32_t exp_type); 

/* General main function */
#define DEFAULT_EXP 1
#define GEN_ONLY 2

#define INFINITESIMAL_TIME 0.000000000001

#endif

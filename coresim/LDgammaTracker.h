#ifndef SIMULATOR_LDGAMMATRACKER_H
#define SIMULATOR_LDGAMMATRACKER_H

#include <vector>
#include "GammaTracker.h"
#include "TelemetryTracker.h"
#include <array>

class Flow;
class Packet;
class Queue;


extern u_int64_t telemetryStatistics[15][28];
extern u_int64_t basicAlgorithmStatistics[6][4];
extern u_int64_t algorithm2Statistics[30][4];


//extern u_int64_t arrDel[15][28];



class LDgammaTracker {

public:

    LDgammaTracker(Flow* flow, int size);
    void notifyPacketEnque(Packet* p, Queue* queue);
    void notifyPacketDrop(Packet* p, Queue* queue);
    void finalizeResults();

    constexpr static std::array<int, 6> Ns = {5, 5, 6, 6, 7, 7};
    constexpr static std::array<int, 6> Ts = {2, 3, 2, 3, 2, 3};

    constexpr static std::array<int, 4> telemetry_t = {2, 3, 3, 3};
    constexpr static std::array<int, 4> telemetry_gamma = {2, 6, 5, 4};


private:
    int num_packets,size;
    int last_arrived_packet;
    Flow* flow;
    std::vector<std::pair<int, int>> dropped_packets;
    int R;
    std::vector<GammaTracker> gammaTrackers;





    std::vector<TelemetryTracker> telemetryTrackers;
};


#endif //SIMULATOR_LDGAMMATRACKER_H

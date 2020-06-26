#include <assert.h>
#include <sstream>
#include <iostream>
#include "LDgammaTracker.h"
#include "packet.h"


constexpr std::array<int, 6> LDgammaTracker::Ns, LDgammaTracker::Ts;
constexpr std::array<int, 4> LDgammaTracker::telemetry_t, LDgammaTracker::telemetry_gamma;

uint64_t telemetryStatistics[15][28];
uint64_t basicAlgorithmStatistics[6][4];
uint64_t algorithm2Statistics[30][4];

LDgammaTracker::LDgammaTracker(Flow *flow, int size) {
    this->size = size;
    this->flow = flow;
    this->last_arrived_packet = -1;
    this->num_packets = 0;
    this->R = 0;

    for (int i = 0; i < Ns.size(); i++)
    {
        gammaTrackers.emplace_back(Ns[i], Ts[i]);
    }

    for (int i = 0; i < 15; i++) {
        for (int j = 0; j < telemetry_t.size(); j++) {
            telemetryTrackers.emplace_back(5 + i * 2, telemetry_t[j], telemetry_gamma[j], size);
        }
    }

}


void LDgammaTracker::notifyPacketEnque(Packet *p, Queue *queue) {
    if (queue != nullptr)
    {
        if (queue->src->type == HOST)
            p->packet_number_by_first_switch = num_packets++;
        return;
    }

    assert(p->packet_number_by_first_switch != last_arrived_packet);
    if (p->packet_number_by_first_switch > last_arrived_packet)
    {
        last_arrived_packet = p->packet_number_by_first_switch;
    }
    else
    {
        R = std::max(R, last_arrived_packet - p->packet_number_by_first_switch);
    }

    for (auto& tracker : gammaTrackers)
    {
        tracker.update(p->packet_number_by_first_switch);
    }

    for (auto& tracker : telemetryTrackers)
    {
        tracker.update(p->packet_number_by_first_switch);
    }

}

void LDgammaTracker::notifyPacketDrop(Packet *p, Queue *queue) {
    int insertion_pos = -1;
    for (int i = 0; i < dropped_packets.size(); i++) {
        if (dropped_packets[i].second + 1 == p->packet_number_by_first_switch) {
            dropped_packets[i].second++;
            insertion_pos = i;
            break;
        }
    }
    if (insertion_pos == -1) {
        insertion_pos = dropped_packets.size();
        dropped_packets.emplace_back(p->packet_number_by_first_switch, p->packet_number_by_first_switch);
    }

    for (int i = 0; i < dropped_packets.size(); i++) {
        if (dropped_packets[insertion_pos].second + 1 == dropped_packets[i].first) {
            dropped_packets[insertion_pos].second = dropped_packets[i].second;
            dropped_packets.erase(dropped_packets.begin() + i);
            break;
        }
    }
    p->flow->receive(p);

}

void LDgammaTracker::finalizeResults() {
    int L = 0;
    for (const auto &seg : dropped_packets) {
        L = std::max(L, seg.second - seg.first + 1);
    }

    {   // Algorithm 1 statistics
        for (int i = 0; i < Ns.size(); i++) {
            if (num_packets < (1 << Ns[i])) {
                continue;
            }
            basicAlgorithmStatistics[i][3]++;
            bool isValid = (R + L < (1 << (Ns[i] - 1))) && (R <= (1 << (Ns[i] - 1)) - (1 << (Ns[i] - Ts[i])));
            if (!isValid) {
                basicAlgorithmStatistics[i][0]++;
            }
            int val = gammaTrackers[i].getValue(num_packets, (1 << (Ts[i]-1)));
            if (val != num_packets) {
                basicAlgorithmStatistics[i][1]++;
            }
            if (val > 1.02* num_packets || val < num_packets - gammaTrackers[i].delivered) {
                basicAlgorithmStatistics[i][2]++;
            }

        }

    }

    {  //  Algorithm 2 statistics
        int globID = -1;
        for (int i = 0; i < Ns.size(); i++) {
            for (int gamma = 1; gamma <= ((1 << Ts[i]) - 1); gamma++) {
                globID++;
                if (num_packets < (1 << Ns[i])) {
                    continue;
                }
                algorithm2Statistics[globID][3]++;
                bool isValid = gammaTrackers[i].hasSpan(num_packets, gamma) &&
                               (R <= (1 << Ns[i]) - (gamma + 1) * (1 << (Ns[i] - Ts[i])));
                if (!isValid) {
                    algorithm2Statistics[globID][0]++;
                }
                int val = gammaTrackers[i].getValue(num_packets, gamma);
                if (val != num_packets) {
                    algorithm2Statistics[globID][1]++;
                }
                if (val > 1.02 * num_packets || val < num_packets - gammaTrackers[i].delivered) {
                    algorithm2Statistics[globID][2]++;
                }
            }
        }
    }

    {   // Telemetry statistics
        for (int i = 0; i < telemetryTrackers.size(); i++) {
            int x = i / 4;
            int y = i % 4 * 7;
            if (num_packets < 8 * 33 + 1) {
                continue;
            }

            telemetryStatistics[x][y + 6]++;
            bool isValid = (R <= ((1 << telemetry_t[i % 4]) - telemetry_gamma[i % 4] - 1) * (5 + x * 2)) &&
                           (R + L < telemetry_gamma[i % 4] * (5 + x * 2));
            if (!isValid) {
                telemetryStatistics[x][y + 4]++;
            }
            if (!telemetryTrackers[i].isCorrect()) {
                telemetryStatistics[x][y + 5]++;
            }

            if (!telemetryTrackers[i - i % 4].isCorrect()) {
                continue;
            }
            std::tuple<int, int, int, int> stat = telemetryTrackers[i].getDelayInfo();
            telemetryStatistics[x][y] += (u_int64_t) std::get<0>(stat);
            telemetryStatistics[x][y + 1] += (u_int64_t) std::get<1>(stat);
            telemetryStatistics[x][y + 2] += (u_int64_t) std::get<2>(stat);
            telemetryStatistics[x][y + 3] += (u_int64_t) std::get<3>(stat);

        }
    }

    if (num_packets != size) {
        while (true) {}
    }
}
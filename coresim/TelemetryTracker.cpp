#include <sstream>
#include <iostream>
#include "TelemetryTracker.h"

#define LEN 10

TelemetryTracker::TelemetryTracker(int G_, int t_, int gamma_, int flow_size_)
    : G(G_),
      t(t_),
      gamma(gamma_),
      beta((1<<t) - 1 - gamma_),
      c2(0),
      is_wrong(false),
      numberOfNonEndingGroups(0),
      totalDelayOfNonEndingGroups(0),
      totalDelayOfOptimiziedNonEndingGroups(0),
      numberOfNonEndingOptimizedGroups(0),
      lag(LEN, 0),
      numArrivedPackets(LEN, 0),
      flow_size(flow_size_)
      {};

void TelemetryTracker::update(int num)
{
    if (is_wrong) {
        return;
    }

    for (int i = 0; i <= beta; i++) {
        if (c2 >= i) {
            lag[(c2-i)%LEN]++;
        };
    }

    int mod = (1 << t) - 1;
    int mid = (num / G) & mod;
    int diff = (mid - (c2 & mod) + 3*mod + 3) & mod;

    int calculated_group;
    if (diff <= gamma) {
        for (int i = 0; i < diff; i++) {
            if (c2-beta >= 0) {
                updateGroupCounters(c2-beta);
            }
            c2++;
            lag[c2%LEN] = 0;
            numArrivedPackets[c2%LEN] = 0;
        }
        calculated_group = c2;
    } else {
        calculated_group = c2 - ((mod+1 - diff) & mod);
    }
    if (calculated_group < c2 - beta) {
        while (true) {};
    }


    lag[calculated_group % LEN] = 0;
    numArrivedPackets[calculated_group % LEN]++;
    is_wrong |= (calculated_group != (num/G));
}



void TelemetryTracker::updateGroupCounters(int i) {
    if (i + gamma < flow_size/G) {
        numberOfNonEndingGroups++;
        totalDelayOfNonEndingGroups += lag[i % LEN];
        totalDelayOfOptimiziedNonEndingGroups += (numArrivedPackets[i%LEN] == G ? 0 : lag[i%LEN]);
        numberOfNonEndingOptimizedGroups += (numArrivedPackets[i%LEN] == G ? 0 : 1);
    }
}

std::tuple<int, int, int, int> TelemetryTracker::getDelayInfo() {
    return std::tuple<int, int, int, int>(numberOfNonEndingGroups, totalDelayOfNonEndingGroups, totalDelayOfOptimiziedNonEndingGroups, numberOfNonEndingOptimizedGroups);
}

bool TelemetryTracker::isCorrect() {
    return !is_wrong;
}




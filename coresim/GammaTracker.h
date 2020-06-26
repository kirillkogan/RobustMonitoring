#ifndef SIMULATOR_GAMMATRACKER_H
#define SIMULATOR_GAMMATRACKER_H


#include <vector>
#include <string>

class GammaTracker {
public:
    GammaTracker(int n, int t);
    void update(int num);
    int getValue(int size, int i);
    bool hasSpan(int size, int gamma);
    int delivered;


private:
    int n, t;
    std::vector<int> curNs;
    std::vector<int> curC2s;
    int last_update_num;

};


#endif //SIMULATOR_GAMMATRACKER_H

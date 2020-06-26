#include "GammaTracker.h"

GammaTracker::GammaTracker(int n_, int t_): n(n_), t(t_), curNs((1 << t)-1, 0), curC2s((1<<t)-1, 0), delivered(0), last_update_num(0) {}

void GammaTracker::update(int num)
{
    delivered++;
    for (int gamma = 1; gamma < (1 << t); gamma++)
    {
        int& curN = curNs[gamma-1];
        if (num > curN && ((num >> (n- t)) - (curN >> (n-t))) <= gamma)
        {
            curN = num;
        }

        int mod = (1 << t) - 1;
        int mid = (num >> (n-t)) & mod;
        int diff = (mid - (curC2s[gamma-1] & mod) + 3*mod + 3) & mod;
        if (diff <= gamma)
        {
            last_update_num = num;
            curC2s[gamma-1] += diff;
        }
    }
}

bool GammaTracker::hasSpan(int size, int gamma){
    return curNs[gamma-1] - (curNs[gamma-1]%(1<<(n-t))) + (1 << n) > size;
}

int GammaTracker::getValue(int size, int gamma){
    int mod = 1 << n;
    int val = curC2s[gamma - 1] << (n - t);
    int diff = (-val % mod + size % mod + 3*mod) % mod;
    return val+diff;
}
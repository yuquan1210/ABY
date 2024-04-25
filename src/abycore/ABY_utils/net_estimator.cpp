#include <sys/time.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "comm_track.h"
#include <map>

void estimate_parameters(std::map<uint32_t, double> timings, std::map<uint32_t, uint8_t> datasent, int num_layers, double& RTT, double& bw) {
    double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    int n = num_layers;

    for(int layer = 0; layer < num_layers; layer++){
        sum_x += datasent[layer];
        sum_y += timings[layer];
        sum_xx += datasent[layer] * datasent[layer];
        sum_xy += datasent[layer] * timings[layer];
    }

    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    RTT = (sum_y - slope * sum_x) / n;
    bw = 1 / slope;  // because slope = 1/bw
}
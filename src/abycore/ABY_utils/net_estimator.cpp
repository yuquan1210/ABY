#include <sys/time.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "net_estimator.h"
#include <map>

void estimate_network(std::map<uint32_t, double> send_timings, std::map<uint32_t, uint64_t> send_datasize, std::map<uint32_t, double> rcv_timings, std::map<uint32_t, uint64_t> rcv_datasize, int num_layers, double& RTT, double& bw) {
    double sum_x = 0, sum_y = 0, sum_xx = 0, sum_xy = 0;
    int n = num_layers;

    double datasize_layer;
    double timing_layer;
    for(int layer = 0; layer < num_layers; layer++){
        datasize_layer = send_datasize[layer]*8 +rcv_datasize[layer]*8; //byte to bits, times 8
        timing_layer = send_timings[layer] + rcv_timings[layer]; 
        sum_x += datasize_layer;
        sum_y += timing_layer;
        sum_xx += datasize_layer * datasize_layer;
        sum_xy += datasize_layer * timing_layer;
    }

    double slope = (n * sum_xy - sum_x * sum_y) / (n * sum_xx - sum_x * sum_x);
    RTT = (sum_y - slope * sum_x) / n;
    bw = 1 / slope;  // because slope = 1/bw
}
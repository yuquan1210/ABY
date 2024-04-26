
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>

void estimate_network(std::map<uint32_t, double> send_timings, std::map<uint32_t, uint64_t> send_datasize, std::map<uint32_t, double> rcv_timings, std::map<uint32_t, uint64_t> rcv_datasize, int num_layers, double& RTT, double& bw);
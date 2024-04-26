
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>

extern std::vector<std::vector<double>> localop_timings;
extern std::vector<std::vector<double>> interop_timings;
extern std::vector<std::vector<double>> finishlay_timings;
extern std::map<uint32_t, double> send_timings;
extern std::map<uint32_t, uint64_t> send_datasize;
extern std::map<uint32_t, double> rcv_timings;
extern std::map<uint32_t, uint64_t> rcv_datasize;

void SaveLocalOpTiming(double timing, int layer);
void SaveInterOpTiming(double timing, int layer);
void SaveFinishLayerTiming(double timing, int layer);
void SaveSendTiming(uint32_t layer, double timing);
void SaveSendDataSize(uint32_t layer, uint64_t datasize);
void SaveRcvTiming(uint32_t layer, double timing);
void SaveRcvDataSize(uint32_t layer, uint64_t datasize);
void clearTimings();


inline std::vector<std::vector<double>> GetLocalOpTimingsFromTimer() {
	return localop_timings;
}

inline std::vector<std::vector<double>> GetInterOpTimingsFromTimer() {
	return interop_timings;
}

inline std::vector<std::vector<double>> GetFinishLayerTimingsFromTimer() {
	return finishlay_timings;
}

inline std::map<uint32_t, double> GetSendTimingsFromTimer() {
	return send_timings;
}

inline std::map<uint32_t, uint64_t> GetSendDataSizeFromTimer() {
	return send_datasize;
}

inline std::map<uint32_t, double> GetRcvTimingsFromTimer() {
	return rcv_timings;
}

inline std::map<uint32_t, uint64_t> GetRcvDataSizeFromTimer() {
	return rcv_datasize;
}
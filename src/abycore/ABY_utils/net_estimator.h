
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <map>

extern std::vector<std::vector<double>> localop_timings;
extern std::vector<std::vector<double>> interop_timings;
extern std::vector<std::vector<double>> finishlay_timings;
extern std::map<uint32_t, double> interaction_timings;
extern std::map<uint32_t, uint8_t> interaction_datasent;

void SaveLocalOpTiming(double timing, int layer);
void SaveInterOpTiming(double timing, int layer);
void SaveFinishLayerTiming(double timing, int layer);
void SaveInteractionTiming(uint32_t layer, double timing);
void SaveInteractionDataSent(uint32_t layer, uint8_t datasize);
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

inline std::map<uint32_t, double> GetInteractionTimingsFromTimer() {
	return interaction_timings;
}

inline std::map<uint32_t, uint8_t> GetInteractionDataSentFromTimer() {
	return interaction_datasent;
}
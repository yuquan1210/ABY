
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

extern std::vector<std::vector<double>> localop_timings;
extern std::vector<std::vector<double>> interop_timings;
extern std::vector<std::vector<double>> finishlay_timings;
extern std::vector<double> interaction_timings;

void SaveLocalOpTiming(double timing, int layer);
void SaveInterOpTiming(double timing, int layer);
void SaveFinishLayerTiming(double timing, int layer);
void SaveInteractionTiming(double timing);
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

inline std::vector<double> GetInteractionTimingsFromTimer() {
	return interaction_timings;
}

/**
 \file 		timer.cpp
 \author 	michael.zohner@ec-spride.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2019 ENCRYPTO Group, TU Darmstadt
			This program is free software: you can redistribute it and/or modify
            it under the terms of the GNU Lesser General Public License as published
            by the Free Software Foundation, either version 3 of the License, or
            (at your option) any later version.
            ABY is distributed in the hope that it will be useful,
            but WITHOUT ANY WARRANTY; without even the implied warranty of
            MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
            GNU Lesser General Public License for more details.
            You should have received a copy of the GNU Lesser General Public License
            along with this program. If not, see <http://www.gnu.org/licenses/>.
 \brief		timer Implementation
 */

#include <sys/time.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <vector>

#include "comm_track.h"
#include <map>

std::vector<std::vector<double>> localop_timings(5, std::vector<double>());
std::vector<std::vector<double>> interop_timings(5, std::vector<double>());
std::vector<std::vector<double>> finishlay_timings(5, std::vector<double>());
std::map<uint32_t, double> interaction_timings;
std::map<uint32_t, uint8_t> interaction_datasent;

void SaveLocalOpTiming(double timing, int sharing) {
	localop_timings[sharing].push_back(timing);
}

void SaveInterOpTiming(double timing, int sharing) {
	interop_timings[sharing].push_back(timing);
}

void SaveFinishLayerTiming(double timing, int sharing) {
	finishlay_timings[sharing].push_back(timing);
}

void SaveInteractionTiming(uint32_t layer, double timing) {
	interaction_timings[layer] = timing;
}

void SaveInteractionDataSent(uint32_t layer, uint8_t datasize) {
	interaction_datasent[layer] = datasize;
}

void clearTimings(){
	for (int i = 0; i < 5; i++){
		localop_timings[i].clear();
		interop_timings[i].clear();
		finishlay_timings[i].clear();
	}
	interaction_timings.clear();
	interaction_datasent.clear();
}

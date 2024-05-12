/**
 \file 		millionaire_prob.cpp
 \author 	sreeram.sadasivam@cased.de
 \copyright	ABY - A Framework for Efficient Mixed-protocol Secure Two-party Computation
			Copyright (C) 2019 Engineering Cryptographic Protocols Group, TU Darmstadt
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
 \brief		Implementation of the millionaire problem using ABY Framework.
 */

#include "common.h"
#include "../../../abycore/circuit/booleancircuits.h"
#include "../../../abycore/sharing/sharing.h"

int32_t test_millionaire_prob_circuit(e_role role, const std::string& address, uint16_t port, seclvl seclvl,
		uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg, e_sharing sharing) {

	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads,
			mt_alg);

	std::vector<Sharing*>& sharings = party->GetSharings();

    Circuit* ac = sharings[S_ARITH]->GetCircuitBuildRoutine();
	Circuit* yc = sharings[S_YAO] ->GetCircuitBuildRoutine();
	Circuit* bc = sharings[S_BOOL]->GetCircuitBuildRoutine();
	Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine();

	share *s_alice_money, *s_bob_money, *s_out;

	uint32_t alice_money, bob_money, output;
	srand(time(NULL));
	alice_money = rand();
	bob_money = rand();

	if(role == SERVER) {
		s_alice_money = ac->PutDummyINGate(bitlen);
		s_bob_money = ac->PutINGate(bob_money, bitlen, SERVER);
	} else { //role == CLIENT
		s_alice_money = ac->PutINGate(alice_money, bitlen, CLIENT);
		s_bob_money = ac->PutDummyINGate(bitlen);
	}

	s_out = BuildMillionaireProbCircuit(s_alice_money, s_bob_money, ac, bc, yc);

	SaveMaxDepth(party->GetTotalDepth());
	GenerateCircSpreadSheet("ss.csv");
	GenerateCircGraph("graph.csv");
    CalculateAllCircuitCost(ac, bc, yc);
	party->ExecCircuit();

	output = s_out->get_clear_value<uint32_t>();

	std::vector<std::vector<double>> tt_localop_timings(5);
    std::vector<std::vector<double>> tt_interop_timings(5);
    std::vector<std::vector<double>> tt_finishlay_timings(5);
    std::map<uint32_t, double> tt_send_timings;
    std::map<uint32_t, uint64_t> send_datasize;
    std::map<uint32_t, double> tt_rcv_timings;
    std::map<uint32_t, uint64_t> rcv_datasize;
    double tt_online_time = 0.0;
    double online_time;
	std::vector<std::vector<double>> localop_timings = party->GetLocalOpTimings();
	std::vector<std::vector<double>> interop_timings = party->GetInterOpTimings();
	std::vector<std::vector<double>> finishlay_timings = party->GetFinishLayerTimings();
	std::map<uint32_t, double> send_timings = party->GetSendTimings();
    std::map<uint32_t, double> rcv_timings = party->GetRcvTimings();
	for(int sharing = 0; sharing < 5; sharing++){
		tt_localop_timings[sharing] = localop_timings[sharing];
		tt_interop_timings[sharing] = interop_timings[sharing];
		tt_finishlay_timings[sharing] = finishlay_timings[sharing];
	}
	tt_send_timings = send_timings;
    tt_rcv_timings = rcv_timings;
	online_time = party->GetTiming(P_ONLINE);
	tt_online_time += online_time;
	std::cout << "Average Online time distribution: " << std::endl;
	// std::cout << "Bool: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[0][layer] / num_tests << ", interactive gate: " << tt_interop_timings[0][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[0][layer] / num_tests << std::endl;
    // }
    // std::cout << "Yao Server: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[1][layer] / num_tests << ", interactive gate: " << tt_interop_timings[1][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[1][layer] / num_tests << std::endl;
    // }
	// std::cout << "Yao Receiver: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[3][layer] / num_tests << ", interactive gate: " << tt_interop_timings[3][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[3][layer] / num_tests << std::endl;
    // }
    // std::cout << "Arith: " << std::endl;
    // for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
    //     std::cout << "\tlayer " << layer << ": local gate: " << tt_localop_timings[2][layer] / num_tests << ", interactive gate: " << tt_interop_timings[2][layer] / num_tests << ", layer finish: " << tt_finishlay_timings[2][layer] / num_tests << std::endl;
    // }
	std::cout << "Communication: " << std::endl;
    for(int layer = 0; layer < tt_localop_timings[0].size(); layer++){
        std::cout << "Total: " << send_datasize[layer]+rcv_datasize[layer] << " bytes " << tt_send_timings[layer] + tt_rcv_timings[layer] << " ms, "
        << "Send: " << send_datasize[layer] << " bytes " << tt_send_timings[layer] << " ms, "
        << "Rcv: " << rcv_datasize[layer] << " bytes " << tt_rcv_timings[layer] << " ms" << std::endl;
    }
    std::cout << std::endl;
    std::cout << "\ntt_online_time: " << tt_online_time << " ms" << std::endl;
    // std::cout << "avg online time over " << num_tests << " reps: " << tt_online_time / ((double)num_tests) << std::endl;
	delete party;
	return 0;
}

share* BuildMillionaireProbCircuit(share *s_alice, share *s_bob, Circuit *ac, Circuit *bc, Circuit *yc) {
    share* mul_out_a;
	share* mul_out_y;
	share* and_out_y;

	/** Calling the greater than equal function in the Boolean circuit class.*/
    mul_out_a = ac->PutMULGate(s_alice, s_bob);
	mul_out_y = yc->PutA2YGate(mul_out_a);
	and_out_y = yc->PutANDGate(mul_out_y, mul_out_y);

	return bc->PutOUTGate(and_out_y, ALL);
}

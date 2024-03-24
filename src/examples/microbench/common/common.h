#pragma once

#include <abycore/circuit/booleancircuits.h>
#include <abycore/circuit/arithmeticcircuits.h>
#include <abycore/circuit/circuit.h>
#include <abycore/aby/abyparty.h>
#include <math.h>
#include <cassert>
#include "cost_calculator.h"

share* PutTheGateForBenchmark(share* a, share* b, Circuit* circ, 
	uint32_t bitlen=32);
uint32_t PutTheOpToVerifyResult(uint32_t a, uint32_t b);

int32_t test_circuit(e_role role, char* address, uint16_t port, seclvl seclvl,
		uint32_t nvals, uint32_t bitlen, uint32_t nthreads, e_mt_gen_alg mt_alg,
		e_sharing sharing, uint32_t num_tests, uint32_t num_non_amortized, uint32_t seed);

share* BuildCircuit(share *s_alice, share* s_bob, Circuit *bc);

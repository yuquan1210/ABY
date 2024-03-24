#include "../common/common.h"

share* PutTheGateForBenchmark(share* a, share* b, Circuit* circ, 
	uint32_t bitlen) {

	BooleanCircuit* bc = dynamic_cast<BooleanCircuit*>(circ);
	assert(bc != NULL);

	share *gt = bc->PutGTGate(a, b);
	share *eq = bc->PutEQGate(a, b);
	share *ge = bc->PutORGate(gt, eq);
	return ge;
}
uint32_t PutTheOpToVerifyResult(uint32_t a, uint32_t b) {
	return a >= b;
}
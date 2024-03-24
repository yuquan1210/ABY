#include "../common/common.h"

share* PutTheGateForBenchmark(share* a, share* b, Circuit* circ, 
	uint32_t bitlen) {
	BooleanCircuit* bc = dynamic_cast<BooleanCircuit*>(circ);
	assert(bc != NULL);
	return bc->PutORGate(a, b);
}
uint32_t PutTheOpToVerifyResult(uint32_t a, uint32_t b) {
	return (a | b);
}
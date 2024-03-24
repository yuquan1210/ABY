#include "../common/common.h"

share* PutTheGateForBenchmark(share* a, share* b, Circuit* circ, 
	uint32_t bitlen) {
	BooleanCircuit* bc = dynamic_cast<BooleanCircuit*>(circ);
	assert(bc != NULL);
	share *ngt = bc->PutINVGate(circ->PutGTGate(a, b));
	share *neq = bc->PutINVGate(circ->PutEQGate(a, b));
	share *lt = bc->PutANDGate(ngt, neq);
	return lt;
}
uint32_t PutTheOpToVerifyResult(uint32_t a, uint32_t b) {
	return a < b;
}
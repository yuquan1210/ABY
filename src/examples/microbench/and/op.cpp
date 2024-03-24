#include "../common/common.h"

share* PutTheGateForBenchmark(share* a, share* b, Circuit* circ, 
	uint32_t bitlen) {
	return circ->PutANDGate(a, b);
}
uint32_t PutTheOpToVerifyResult(uint32_t a, uint32_t b) {
	return a & b;
}
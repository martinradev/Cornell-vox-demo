#version 430

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

layout (std430, binding = 0) buffer InputPrefixSumBuffer_Declaration {
	int inarr[];
};

layout (std430, binding = 2) buffer OutputPrefixSumBuffer_Declaration {
	int outarr[];
};


layout (std430, binding = 1) buffer PrebixBlockBuffer_Declaration {
	int blockArr[];
};


uniform int numValues;
uniform bool storeLastValues;

shared int buf[2048];

void main() {
   
 
	uint vIndex = gl_GlobalInvocationID.x;
	uint tidx = gl_LocalInvocationID.x;
	
	
	int n = 1024;
	
	int pout = 0;
	int pin = 1;
	
	buf[n*pout + tidx] = tidx == 0 ? 0 : inarr[vIndex-1];
	barrier();
	
	//if (vIndex >= numValues) return;
	
	for (int off = 1; off < n; off *= 2) {
		pout = 1-pout;
		pin = 1-pout;

			if (tidx >= off) {
				buf[pout*n + tidx] = buf[pin*n + tidx - off] + buf[pin*n + tidx];
			} else {
				buf[pout*n + tidx] = buf[pin*n + tidx];
			}

		barrier();
	}
	
	int firstValue = 0;
	if (tidx == n-1) {
		firstValue = inarr[vIndex];
	}
	
	outarr[vIndex] = buf[pout*n+tidx];
	
	if (storeLastValues && tidx == n-1) {
		// final thread
		blockArr[gl_WorkGroupID.x] = outarr[vIndex]+firstValue;
	} else if (!storeLastValues && tidx == n-1) {
		blockArr[n] = outarr[vIndex]+firstValue;
	}


}
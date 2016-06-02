
#pragma optionNV(unroll none)

uniform vec4 cubeInfo; // (x,y,z) first cube, (w) cube len
uniform bool isPrefixSumPass; // compute counts

layout (local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

struct Vertex {
	vec4 position; // w type
	vec4 direction;
	vec4 normal;
	vec4 orig_position;
};

layout (std430, binding = 1) buffer VertexBuffer_Declaration {
	Vertex vertices[];
} vertex_buffer;

layout (std430, binding = 2) buffer PrefixSumBuffer_Declaration {
	uint arr[];
} prefix_sum_buffer;

/*
	START TETRAHEDRA MARCHING
*/


const ivec4 tetraIndices[6] = const ivec4[6] (
		ivec4(0, 5, 6, 1),
		ivec4( 4, 6 ,0, 5 ), // ok
		ivec4( 1, 0, 6, 2 ), // ~
		ivec4( 0, 4, 6 ,7 ),
		ivec4( 0, 3, 7, 6 ),
		ivec4( 0, 3, 2, 6)
);

bool intersect(in float f1, in float f2, in vec3 p1,in vec3 p2, out vec4 intersectionPoint) {

	float s1 = sign(f1);
	float s2 = sign(f2);
	
	if ((s1 < 0 && s2 >= 0) || (s2 < 0 && s1 >= 0)) {
		
		intersectionPoint = vec4(0.5 * (p1+p2), 1);
	
		return true;
	}
	return false;
	
}

/*
	END TETRAHEDRA MARCHING
*/


void main() {
   
   vec3 p = cubeInfo.xyz + vec3(gl_GlobalInvocationID) * cubeInfo.w;
	
   uint WIDTH = gl_NumWorkGroups.x * gl_WorkGroupSize.x;
   uint HEIGHT = gl_NumWorkGroups.y * gl_WorkGroupSize.y;
   
   vec3 cubeVertices[] = vec3[](
			p,
			vec3(cubeInfo.w+p.x, p.y, p.z),
			vec3(cubeInfo.w+p.x, p.y, cubeInfo.w+p.z),
			vec3(p.x, p.y, p.z+cubeInfo.w),
			vec3(p.x, p.y+cubeInfo.w, p.z),
			vec3(p.x+cubeInfo.w, p.y+cubeInfo.w, p.z),
			vec3(p.x+cubeInfo.w, p.y+cubeInfo.w, p.z+cubeInfo.w),
			vec3(p.x, p.y+cubeInfo.w, p.z+cubeInfo.w)
	);
	
	float cubeF[] = float[](
		fScene(cubeVertices[0]),
		fScene(cubeVertices[1]),
		fScene(cubeVertices[2]),
		fScene(cubeVertices[3]),
		fScene(cubeVertices[4]),
		fScene(cubeVertices[5]),
		fScene(cubeVertices[6]),
		fScene(cubeVertices[7])
	);
	
	uint threadIndex = ( gl_GlobalInvocationID.x + WIDTH * (gl_GlobalInvocationID.z * HEIGHT + gl_GlobalInvocationID.y) );
	uint vertexBufferStartIndex = prefix_sum_buffer.arr[threadIndex];
	prefix_sum_buffer.arr[threadIndex] = 0;
	int numTetras = 6;
	for (int tetraId = 0; tetraId < numTetras; ++ tetraId) {
		
			vec3 tetra[] = vec3[](
				cubeVertices[tetraIndices[tetraId].x],
				cubeVertices[tetraIndices[tetraId].y],
				cubeVertices[tetraIndices[tetraId].z],
				cubeVertices[tetraIndices[tetraId].w]
			);
			
			float tetraF[] = float[](
				cubeF[tetraIndices[tetraId].x],
				cubeF[tetraIndices[tetraId].y],
				cubeF[tetraIndices[tetraId].z],
				cubeF[tetraIndices[tetraId].w]
			);
			
			vec4 points[4];
			int foundPoints=0;
			
			for (int t1 = 0; t1 < 4; ++t1) {
				for (int t2 = t1+1; t2 < 4; ++t2) {
					vec4 intersectionPoint;
					if (intersect(tetraF[t1], tetraF[t2], tetra[t1], tetra[t2], intersectionPoint)) {
						points[foundPoints++] = intersectionPoint;
					}
				}
			}
			
			if (isPrefixSumPass) {
				
				if (foundPoints == 3) {
					prefix_sum_buffer.arr[threadIndex] += 3;
				} else if (foundPoints == 4) {
					prefix_sum_buffer.arr[threadIndex] += 6;
				}
				
			} else {
				if (foundPoints == 3) {
				
					// filter types
					
					vec3 sn = getNormal((points[0].xyz+points[1].xyz+points[2].xyz)/3.0);

					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];
					vertex_buffer.vertices[vertexBufferStartIndex].normal.xyz = sn;
					
					vertex_buffer.vertices[vertexBufferStartIndex+1].position = points[1];
					vertex_buffer.vertices[vertexBufferStartIndex+1].normal.xyz = sn;
					
					vertex_buffer.vertices[vertexBufferStartIndex+2].position = points[2];
					vertex_buffer.vertices[vertexBufferStartIndex+2].normal.xyz = sn;
					
					vertexBufferStartIndex+=3;
				} else if (foundPoints == 4) {
					
					vec3 sn1 = getNormal((points[0].xyz+points[2].xyz+points[3].xyz)/3.0);
					//vec3 sn2 = getNormal((points[0].xyz+points[1].xyz+points[3].xyz)/3.0);
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];
					vertex_buffer.vertices[vertexBufferStartIndex].normal.xyz = sn1;
					
					vertex_buffer.vertices[vertexBufferStartIndex+1].position = points[2];
					vertex_buffer.vertices[vertexBufferStartIndex+1].normal.xyz = sn1;
					
					vertex_buffer.vertices[vertexBufferStartIndex+2].position = points[3];
					vertex_buffer.vertices[vertexBufferStartIndex+2].normal.xyz = sn1;

					vertexBufferStartIndex+=3;
					
					vertex_buffer.vertices[vertexBufferStartIndex].position = points[0];
					vertex_buffer.vertices[vertexBufferStartIndex].normal.xyz = sn1;
					
					vertex_buffer.vertices[vertexBufferStartIndex+1].position = points[3];
					vertex_buffer.vertices[vertexBufferStartIndex+1].normal.xyz = sn1;
					
					vertex_buffer.vertices[vertexBufferStartIndex+2].position = points[1];
					vertex_buffer.vertices[vertexBufferStartIndex+2].normal.xyz = sn1;

					vertexBufferStartIndex+=3;
					
				}
			}
			
			
		}
   
   
   
   
   
   

}
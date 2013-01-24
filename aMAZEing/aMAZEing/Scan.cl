


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Scan_Naive(const __global uint* inArray, __global uint* outArray, uint N, uint offset) 
{
	uint GID = get_global_id(0);

	if(GID > N) {
		return;
	}

	if(GID < offset) {

		if(GID == 0) {
			outArray[GID] = 0;
		} else {
			outArray[GID] = inArray[GID];
		}
		
	} else {
		
		if(offset == 1 && GID == 1) {
			outArray[GID] = inArray[0];
		} else {
			outArray[GID] = inArray[GID] + inArray[GID-offset];
		}

		
	}




}



// Some usefull defines
#define NUM_BANKS			32
#define NUM_BANKS_LOG		5
#define SIMD_GROUP_SIZE		32

// Bank conflicts
#define AVOID_BANK_CONFLICTS
#ifdef AVOID_BANK_CONFLICTS
	
#else
	#define OFFSET(A) (A)
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__kernel void Scan_WorkEfficient(__global uint* array, __global uint* higherLevelArray, __local uint* localBlock) 
{
	uint GID = get_global_id(0);
	uint LID = get_local_id(0);
	uint Lsize = get_local_size(0);
	uint Gsize = get_global_size(0);
	uint offset = 1;

	uint a = 2 * LID;
	uint b = 2 * LID +1;

	a += a/NUM_BANKS;
	b += b/NUM_BANKS;

	//load in localblock
	localBlock[a] = array[2 * GID];
	localBlock[b] = array[2 * GID +1];

	//Up-sweep
	for(uint i = Lsize; i > 0; i/=2) {
		barrier(CLK_LOCAL_MEM_FENCE);

		if(LID < i) {
			a = offset * (2*LID +1) -1;
			b = a + offset;

			//avoid bank conflicts
			a += a/NUM_BANKS;
			b += b/NUM_BANKS;

			localBlock[b] += localBlock[a];
		}
		offset *= 2;
	}
	

	//set last element to 0
	if(LID == 0) {
		a = Lsize * 2 - 1;
		
		//avoid bank conflicts
		a += a/NUM_BANKS;

		localBlock[a] = 0;
	}

	
	//Down-sweep
	for(uint j = 1; j <= Lsize; j*=2) {
		offset /= 2;
		barrier(CLK_LOCAL_MEM_FENCE);

		if(LID < j) {
			
			a = offset * (2*LID +1) -1;
			b = a + offset;

			a += a/NUM_BANKS;
			b += b/NUM_BANKS;
			
			uint tmp = localBlock[a];
			localBlock[a] = localBlock[b];
			localBlock[b] += tmp; 
		}

	}
	
	uint tmp1,tmp2;
	//Write data back and add them to orginaldata for inclusiv PPS
	
	//No need
	//barrier(CLK_LOCAL_MEM_FENCE);
	
	a = 2 * LID;
	b = 2 * LID +1;

	//avoid bank conflicts
	a += a/NUM_BANKS;
	b += b/NUM_BANKS;

	////Read - Write - Read - Write global memory is not that good I think :)
	//array[2*GID] = localBlock[a];
	//array[2*GID+1] = localBlock[b];

	//now its Read - Read - Write - Write Global memory
	//tmp1 = array[2*GID] + localBlock[a];
	//tmp2 = array[2*GID+1] + localBlock[b];

	tmp1 = localBlock[a];
	tmp2 = localBlock[b];

		//write last element to higherLevelArray for GROUP PPS
	if(LID == Lsize -1) {
		int workgroup = (GID+1 - Lsize)  / Lsize;
		higherLevelArray[workgroup] = tmp2 + array[2*GID+1]; 
	}

	array[2*GID] = tmp1;
	array[2*GID+1] = tmp2;





}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Kernel that should add the group PPS to the local PPS (Figure 14)
__kernel void Scan_WorkEfficientAdd(__global uint* higherLevelArray, __global uint* array, __local uint* localBlock) 
{
	uint GID = get_global_id(0);
	uint LID = get_local_id(0);
	uint Lsize = get_local_size(0);

	//first block has already the right values 
	uint blockSize =  2 * get_local_size(0);
	if(GID < blockSize) {
		return;
	}

	//Load elements for this block in shared memory from array
	localBlock[LID] = array[GID];


	//Sum every entry in localBlock to whole block in array

	localBlock[LID] += higherLevelArray[ (GID / blockSize)];

	//write back data in array
	array[GID] = localBlock[LID];

}
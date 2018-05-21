/*
 * ocl.h
 *
 *  Created on: Apr 6, 2018
 *      Author: Like.Z
 */

#ifndef OCL_H_
#define OCL_H_
#include <CL/cl.h>
#include "/Hash/Hash.h"

#define _MAX_DEVICES_NUM 64

typedef struct {

	union{
		cl_platform_id platform;
		cl_context context;
		cl_uint total;
		cl_uint left;
		cl_uint offset;
		cl_device_id device;
	} platform[4][_MAX_DEVICES_NUM + 1];	/*	[0][0]   : total number
												[1][0]   : left capability
												[0][1..*]: cl_platform_id
												[1][1..*]: cl_context
												[2][1..*]: cl_device_id*
												[3][1..*]: number of devices for each platform
											 */
	union{
		cl_device_id device;
		cl_program program;
		cl_uint offset;
		cl_uint total;
		cl_uint left;
	} device[4][_MAX_DEVICES_NUM+1];			/*	[0][0]   : total number
													[1][0]   : left capability
													[0][1..*]: cl_device_id
													[1][1..*]: program
													[2][1..*]: cl_device_id* (subdevice)
													[3][1..*]: number of sub devices for each device
												*/
	union{
		cl_device_id device;
		cl_program program;
		cl_uint total;
		cl_uint left;
	} subDevice[2][_MAX_DEVICES_NUM+1];		/*	[0][0]   : total number
												[1][0]   : left capability
												[0][1..*]: cl_device_id
												[1][1..*]: program
											*/
	union{
		cl_kernel* kernel;
		cl_uint total;
	} kernel[_MAX_DEVICES_NUM+1];			//one kernel array for only one context(one platform)
	HashTable kernelMap;

	//	void* svm_ptr;
	//	cl_context context[_MAX_DEVICES_NUM+1];			//one context for only one platform
	//	cl_command_queue cmdQueue[_MAX_DEVICES_NUM+1];	//one cmdQueue for only one device
	//	cl_program program[_MAX_DEVICES_NUM+1];			//one program for only one platform
} Ocl;

typedef struct oclConfig{
	char ** conf;
	const cl_uint item_num;
}OCLConfig;

#endif /* OCL_H_ */

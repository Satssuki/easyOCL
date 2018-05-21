/*
 * esayOcl.c
 *
 *  Created on: May 12, 2018
 *      Author: Like.Z
 */
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <CL/cl.h>
#include "ocl.h"
//static const OCLConfig ocl_cfg_defalut={};

static char *strtok_rr(char *str, const char *delim, char **saveptr){
	char * s=str;
	const char *d;
	while(*s != '\0'){
		d=delim;
		while(*d != '\0'){
			if(*s != *d){
				d++;
			}else{
				if(s != str){
					*s='\0';
					*saveptr=s+1;
					return str;
				}else{
					str++;
					break;
				}
			}
		}
		s++;
	}
	*saveptr=s;
	return str != s?str:NULL;
}

static char * loadFile(const char* fileName){
	FILE* fp=fopen(fileName, "r");
	if (!fp) {
	    fprintf(stderr, "Failed to load kernel.\n");
	    exit(1);
	}
	struct stat tmp_s;
	stat(fileName,&tmp_s);
	int source_size=tmp_s.st_size;

	char *source_str = (char *)malloc(source_size);

	if(source_size!=fread(source_str, 1, source_size, fp)){
		fclose(fp);
		return NULL;
	}
	fclose(fp);
	printf("%s\n",source_str);
	return source_str;
}

static int unLoadFile(char * const source_str){
	free(source_str);
	return 0;
}

Ocl createOcl(const OCLConfig* cfg) {

	cl_uint n=0;
	const cl_uint max_devices_num=_MAX_DEVICES_NUM;
	char buf[2048],stmp[64];
	size_t size;
	cl_int status;


	Ocl ocl;
	ocl.platform[0][0].total=ocl.device[0][0].total=ocl.subDevice[0][0].total=0;
	ocl.platform[1][0].left=ocl.device[1][0].left=ocl.subDevice[1][0].left=max_devices_num;

	// Get the platforms
	status = clGetPlatformIDs(max_devices_num, (cl_platform_id*)ocl.platform[0]+1, &n);

	ocl.platform[0][0].total = max_devices_num < n ? max_devices_num : n;
	ocl.platform[1][0].left = max_devices_num - ocl.platform[0][0].total;

	for(int l=0; l<cfg->item_num && ocl.device[1][0].left>0; ++l){
		char platIdx=cfg->conf[l][0];
		// Get the devices
		char devOffset=ocl.device[0][0].total+1;
		cl_uint j, left_cap=ocl.device[1][0].left;

		status = clGetDeviceIDs(ocl.platform[0][platIdx].platform,
								*(cl_device_type*)(cfg->conf[l]+3),
								left_cap,
								(cl_device_id*)ocl.device[0] + devOffset,
								(&j) );
		j=left_cap<j?left_cap:j;
		ocl.device[1][0].left-=j;
		ocl.device[0][0].total+=j;
		ocl.platform[2][platIdx].offset=devOffset;

		// Create a context associated with the specified devices
		cl_context context=ocl.platform[1][platIdx].context = clCreateContext(NULL,
											j,
											(cl_device_id*)ocl.device[0]+devOffset,
											NULL, NULL,
											&status);

		//devices info fill
		for(int devNum=cfg->conf[l][1];devNum>0;--devNum){
			char devNo=cfg->conf[++l][0],devIdx=devNo-1+devOffset;
			char subDevNum=cfg->conf[l][1];
			cl_device_partition_property* cdpp=(cl_device_partition_property*)(cfg->conf[l]+2);
			char* file=cfg->conf[l]+10;
			if(*file!='\0'){
				char *fa[max_devices_num];
				int i=0;
				while(i<max_devices_num&&(fa[i]=strtok_rr(file,",",&file))!=NULL){
					fa[i]=loadFile(fa[i]);
					i++;
				}
				// build Program;
				cl_program prog=ocl.device[1][devIdx].program = clCreateProgramWithSource(context, i,
							(const char**)fa, NULL, &status);
				while(i>0){
					unLoadFile(fa[--i]);
				}
				status = clBuildProgram(prog, 1, (cl_device_id*)(ocl.device[0]+devIdx), NULL, NULL, NULL);
				//TODO kernel to hashmap
				clGetProgramInfo(prog,CL_PROGRAM_NUM_KERNELS,sizeof(n),&n,NULL);

				cl_kernel* kernels=ocl.kernel[++ocl.kernel[0].total].kernel=malloc(sizeof(cl_kernel)*(n+1));
				kernels[0]=n;

				clGetProgramInfo(prog,CL_PROGRAM_KERNEL_NAMES,sizeof(buf),buf,&size);
				buf[2047]='\0';
				for(char * p=buf,*kname;(kname=strtok_rr(p,",",&p))!=NULL;--n){
					kernels[n]= clCreateKernel(prog, kname, &status);
					if (status != 0) {
						printf("ERROR: kernel name-> %s ,CODE %d\n", kname, status);
					} else {
						char st[]={'@',platIdx,devNo};
						strcat(strcpy(stmp,kname),st);
						ocl.kernelMap.set(&(ocl.kernelMap), kernels[n],
								stmp, strlen(stmp)+1);
					}
				}

			}

			//sub-devices info fill
			if(subDevNum>0 && subDevNum<=ocl.subDevice[1][0].left){
				int subDevOffset=ocl.subDevice[0][0].total+1;
				clCreateSubDevices(ocl.device[0][devIdx].device,
											  cdpp,
											  subDevNum,
											  (cl_device_id*)ocl.subDevice[0]+subDevOffset,
											  &j);
				j=subDevNum<j?subDevNum:j;
				ocl.device[2][devIdx].offset=subDevOffset;
				ocl.device[3][devIdx].total=j;

				ocl.subDevice[0][0].total+=j;
				ocl.subDevice[1][0].left-=j;

				while(0<subDevNum--){
					int subDevNo=cfg->conf[++l][0],subDevIdx=subDevNo-1+subDevOffset;
					char* file=cfg->conf[l]+1;
					if(*file!='\0'){
						char *fa[max_devices_num];
						int i=0;
						while(i<max_devices_num&&(fa[i]=strtok_rr(file,",",&file))!=NULL){
							fa[i]=loadFile(fa[i]);
							i++;
						}
						// build program;
						cl_program prog=ocl.subDevice[1][subDevIdx].program = clCreateProgramWithSource(context, i,
									(const char**)fa, NULL, &status);
						while(i>0){
							unLoadFile(fa[--i]);
						}
						status = clBuildProgram(prog, 1, (cl_device_id*)(ocl.subDevice[0]+devIdx), NULL, NULL, NULL);
						//TODO kernel to hashmap
						clGetProgramInfo(prog,CL_PROGRAM_NUM_KERNELS,sizeof(n),&n,NULL);
						cl_kernel* kernels=ocl.kernel[++ocl.kernel[0].total].kernel=malloc(sizeof(cl_kernel)*(n+1));
						kernels[0]=n;
						clGetProgramInfo(prog,CL_PROGRAM_KERNEL_NAMES,sizeof(buf),buf,&size);
						buf[2047]='\0';
						for(char * p=buf,*kname;(kname=strtok_rr(p,",",&p))!=NULL;--n){
							kernels[n]= clCreateKernel(prog, kname, &status);
							if (status != 0) {
								printf("ERROR: kernel name-> %s ,CODE %d\n", kname, status);
							} else {
								char st[]={'@',platIdx,devNo,subDevNo};
								strcat(strcpy(stmp,kname),st);
								ocl.kernelMap.set(&(ocl.kernelMap), kernels[n],
										stmp, strlen(stmp)+1);
							}
						}

					}
				}
			}

		}
	}

	return ocl;
}




int destroyOcl(Ocl* ocl) {
	int ret = 0;
	cl_int status = 0;
	for (int i = ocl->kernel[0].total; i > 0; --i) {
		for (int j = ocl->kernel[i].kernel[0]; j > 0; --j) {
			if (ocl->kernel[i].kernel[j] != NULL
					&& (status = clReleaseKernel(ocl->kernel[i].kernel[j])))
				ret = -1;
			printf("OCL-Erro:%d\n", status);
		}
	}

	for (int i = ocl->subDevice[0][0].total; i > 0; --i) {
		if (ocl->subDevice[1][i].program != NULL
				&& (status = clReleaseProgram(ocl->subDevice[1][i].program))) {
			ret = -1;
			printf("OCL-Erro:%d\n", status);
		}

		if (ocl->subDevice[0][i].device != NULL
				&& (status = clReleaseDevice(ocl->subDevice[0][i].device))) {
			ret = -1;
			printf("OCL-Erro:%d\n", status);
		}
	}

	for (int i = ocl->device[0][0].total; i > 0; --i) {
		if (ocl->device[1][i].program!=NULL
				&& (status = clReleaseProgram(ocl->device[1][i].program))) {
			ret = -1;
			printf("OCL-Erro:%d\n", status);
		}

		if (ocl->device[0][i].device!=NULL
				&& (status = clReleaseDevice(ocl->device[0][i].device))) {
			ret = -1;
			printf("OCL-Erro:%d\n", status);
		}
	}

	for (int i = ocl->platform[0][0].total; i > 0; --i) {
		if (ocl->platform[1][i].context!=NULL
				&& (status = clReleaseContext(ocl->platform[1][i].context))) {
			ret = -1;
			printf("OCL-Erro:%d\n", status);
		}
	}

//	cl_int status = clReleaseKernel(ocl->kernel);
//	if (status == CL_SUCCESS) {
//		ret = -1;
//		printf("oclErro:%d\n", status);
//	}
//	if (status = clReleaseProgram(ocl->program)) {
//		ret = -1;
//		printf("oclErro:%d\n", status);
//	}
//	if (status = clReleaseCommandQueue(ocl->cmdQueue)) {
//		ret = -1;
//		printf("oclErro:%d\n", status);
//	}
//	//free SVM
////	if (status = clSVMFree(ocl->context, ocl->)) {
////		ret = -1;
////		printf("oclErro:%d\n", status);
////	}
//	//destroy openCL context
//	if (status = clReleaseContext(ocl->context)) {
//		ret = -1;
//		printf("oclErro:%d\n", status);
//	}
	return ret;
}

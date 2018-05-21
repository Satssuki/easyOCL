/*
 * type.h
 *
 *  Created on: Jan 16, 2018
 *      Author: xpc
 */

#ifndef TYPE_H_
#define TYPE_H_
typedef struct {
	int A[2048];
	int B[2048];
	int C[2048];
}Args;

int add(int a,int b){
	return a+b;
}

#endif /* TYPE_H_ */
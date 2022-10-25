#ifndef CKKSENCODER_H
#define CKKSENCODER_H

#include <cuda.h>
#include <SPOGCKKS/ckkscontext.h>
#include <cuPoly/cuda/cudaengine.h>
#include <cuPoly/cuda/dgt.h>
#include <complex.h>
#include <cuComplex.h>
#include <fftw3.h>

void ckks_encode_single(
	GaussianInteger *a,
	std::complex<double> val,
	uint64_t encodingp);

void ckks_decode_single(
	std::complex<double> *val,
	GaussianInteger *a,
	uint64_t encodingp);

void ckks_encode(
	CKKSContext *ctx,
	GaussianInteger *a,
	std::complex<double> *d_val,
	int slots,
	int empty_slots,
	uint64_t encodingp);

void ckks_decode(
	CKKSContext *ctx,
	std::complex<double> *d_val,
	GaussianInteger *a,
	int slots,
	uint64_t encodingp);

void rotate_slots_right(
	CKKSContext *ctx,
	GaussianInteger *d_a,
	int base,
	int rotSlots);

void rotate_slots_left(
	CKKSContext *ctx,
	GaussianInteger *d_a,
	int base,
	int rotSlots);

void conjugate_slots(
	CKKSContext *ctx,
	GaussianInteger *d_a,
	int base);
void fftSpecialInv(complex<double>* vals, const long size) ;
#endif
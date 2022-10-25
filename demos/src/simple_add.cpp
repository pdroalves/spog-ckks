#include <cuPoly/settings.h>
#include <cuPoly/arithmetic/polynomial.h>
#include <SPOGCKKS/ckks.h>
#include <stdlib.h>
#include <NTL/ZZ.h>
#include <cuda_profiler_api.h>
#include <random>
#include <sstream>
#include <string>

NTL_CLIENT

#define NRUNS 100

int main() {
	cudaProfilerStop();

	ZZ q;

	srand(0);
	NTL::SetSeed(to_ZZ(0));

	// Params
    int nphi = 4096;
	int k = 2;
	int kl = 3;

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, -1);// Init CUDA
        
	// FV setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);
	SecretKey *sk = ckks_new_sk(cipher);
	ckks_keygen(cipher, sk);

	/////////////
	// Message //
	/////////////
	std::uniform_real_distribution<double> distribution = std::uniform_real_distribution<double>(0, 10);
	std::default_random_engine generator;
	complex<double> m1 = {distribution(generator), distribution(generator)};
	complex<double> m2 = {distribution(generator), distribution(generator)};

	/////////////
	// Encrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will encrypt");
	cipher_t* ct1 = ckks_encrypt(cipher, &m1);
	cipher_t* ct2 = ckks_encrypt(cipher, &m2);
	cipher_t* ct3 = new cipher_t;
	cipher_init(cipher, ct3);

	//////////
	// Add //
	////////
	cudaEvent_t start, stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
  	float latency = 0;

	cudaDeviceSynchronize();
	cudaCheckError();
  	cudaEventRecord(start, cipher->get_stream());
  	for(int i = 0; i < NRUNS; i++)
		// Encrypted add
		ckks_add(cipher, ct3, ct1, ct2);
	cudaEventRecord(stop, cipher->get_stream());
	cudaCheckError();
	cudaEventSynchronize(stop);
	cudaCheckError();
	cudaEventElapsedTime(&latency, start, stop);
	cudaProfilerStop();
	std::cout << "cudaEvent_T got " << (latency/NRUNS) << " ms" << std::endl;
	
	cipher_free(cipher, ct1);
	cipher_free(cipher, ct2);
	cipher_free(cipher, ct3);
	delete ct1;
	delete ct2;
	delete ct3;
	delete cipher;
	CUDAEngine::destroy();
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
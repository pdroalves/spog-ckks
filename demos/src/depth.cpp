#include <cuPoly/settings.h>
#include <cuPoly/arithmetic/polynomial.h>
#include <SPOGCKKS/ckks.h>
#include <SPOGCKKS/ckkscontext.h>
#include <stdlib.h>
#include <NTL/ZZ.h>
#include <cuda_profiler_api.h>

const float ERRBOUND = 0.005;

long double get_range(int scalingfactor){
	return (long double)(CUDAEngine::RNSPrimes[0] - 1) / ((uint64_t)1 << (scalingfactor + 1));
}
/**
	sequential_mul:

	The intention of this program is to demonstrate the steps required for a
	simple workload of generating a message, encrypt, and execute a lot of
	sequential multiplications. At the end of each iteration it	shall validate
	if the multiplication was successful or if it failed.
 */
int main() {
	cudaProfilerStop();

	ZZ q;

	srand(0);
	NTL::SetSeed(to_ZZ(0));

	// Params
    int nphi = 4096;
	int k = 47;
	int kl = k+1;
	int64_t scalingfactor = 55;

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, scalingfactor);// Init CUDA
        
	// CKKS setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);
	SecretKey *sk = ckks_new_sk(cipher);
	ckks_keygen(cipher, sk);

	/////////////
	// Message //
	/////////////
	int slots = CUDAEngine::N;
	complex<double> *val = new complex<double>[slots];
	for(int i = 0; i < slots; i++)
		val[i] = {1, 1};

	/////////////
	// Encrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will encrypt");
  	cipher_t* ct = ckks_encrypt(cipher, val, slots);

	//////////
	// Mul //
	////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will mul");
	cipher_t *ct_rotation = new cipher_t;
	cipher_t *ct_log = new cipher_t;
	cipher_t *ct_log1minus = new cipher_t;
	cipher_t *ct_sin = new cipher_t;
	cipher_t *ct_cos = new cipher_t;
	cipher_t *ct_exp = new cipher_t;
	cipher_t *ct_sigmoid = new cipher_t;
	cipher_t *ct_inverse = new cipher_t;
	cipher_t *ct_dot = new cipher_t;
	cipher_copy(cipher, ct_rotation, ct);
	cipher_copy(cipher, ct_log, ct);
	cipher_copy(cipher, ct_log1minus, ct);
	cipher_copy(cipher, ct_sin, ct);
	cipher_copy(cipher, ct_cos, ct);
	cipher_copy(cipher, ct_exp, ct);
	cipher_copy(cipher, ct_sigmoid, ct);
	cipher_copy(cipher, ct_inverse, ct);
	cipher_copy(cipher, ct_dot, ct);

	ckks_rotate(cipher, ct_rotation, ct, 1);
	ckks_log(cipher, ct_log, ct);
	ckks_log1minus(cipher, ct_log1minus, ct);
	ckks_sin(cipher, ct_sin, ct);
	ckks_cos(cipher, ct_cos, ct);
	ckks_exp(cipher, ct_exp, ct);
	ckks_sigmoid(cipher, ct_sigmoid, ct);
	ckks_inverse(cipher, ct_inverse, ct);
	ckks_batch_inner_prod(cipher, ct_dot, ct, ct);

	std::cout << "Originally ct had " << ct->level << " levels." << std::endl << std::endl;
	std::cout << "Rotation consumed " << (ct->level - ct_rotation->level) << " levels." << std::endl << std::endl;
	std::cout << "log consumed " << (ct->level - ct_log->level) << " levels." << std::endl << std::endl;
	std::cout << "log1minus consumed " << (ct->level - ct_log1minus->level) << " levels." << std::endl << std::endl;
	std::cout << "sin consumed " << (ct->level - ct_sin->level) << " levels." << std::endl << std::endl;
	std::cout << "cos consumed " << (ct->level - ct_cos->level) << " levels." << std::endl << std::endl;
	std::cout << "exp consumed " << (ct->level - ct_exp->level) << " levels." << std::endl << std::endl;
	std::cout << "sigmoid consumed " << (ct->level - ct_sigmoid->level) << " levels." << std::endl << std::endl;
	std::cout << "inverse consumed " << (ct->level - ct_inverse->level) << " levels." << std::endl << std::endl;
	std::cout << "dot consumed " << (ct->level - ct_dot->level) << " levels." << std::endl << std::endl;

	/////////////
	// Release //
	/////////////
	//
	cipher_free(cipher, ct_rotation);
	cipher_free(cipher, ct_log);
	cipher_free(cipher, ct_log1minus);
	cipher_free(cipher, ct_sin);
	cipher_free(cipher, ct_cos);
	cipher_free(cipher, ct_exp);
	cipher_free(cipher, ct_sigmoid);
	cipher_free(cipher, ct_inverse);
	cipher_free(cipher, ct_dot);
	CUDAEngine::destroy();

	cudaDeviceSynchronize();
	cudaCheckError();
	
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
#include <cuPoly/settings.h>
#include <cuPoly/arithmetic/polynomial.h>
#include <SPOGCKKS/ckks.h>
#include <stdlib.h>
#include <NTL/ZZ.h>
#include <cuda_profiler_api.h>
#include <complex>
#include <sstream>
#include <string>

#define ERRBOUND 0.05

/**
	range:

 */
int main() {
	cudaProfilerStop();

	ZZ q;

	srand(0);
	NTL::SetSeed(to_ZZ(0));

	// Params
    int nphi = 4096;
	int k = 2;
	int kl = 3;
	int64_t scalingfactor = 55;

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, scalingfactor);// Init CUDA
        
	// Cipher setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);
  	SecretKey *sk = ckks_new_sk(cipher);
  	ckks_keygen(cipher, sk);

	double a = 0;
	double b = 0;
	double pace = 0.1;
	poly_t encoded_val;
	poly_init(cipher, &encoded_val);

	// Testing ranges
	for(double val = b; val > -8192; val-=pace){
		complex<double> decoded_val;

		cipher->encodeSingle(&encoded_val, &scalingfactor, val);
		cipher->decodeSingle(&decoded_val, &encoded_val, scalingfactor);

		double diff = abs(val - real(decoded_val));
		if(diff < ERRBOUND)
			std::cout << "Ok for " << val << std::endl;
		else{
			std::cout << "Failed by " << diff << " for " << val << std::endl;
			a = val + pace;
			break;
		}
	}
	for(double val = a; val < 8192; val+=pace){
		complex<double> decoded_val;

		cipher->encodeSingle(&encoded_val, &scalingfactor, val);
		cipher->decodeSingle(&decoded_val, &encoded_val, scalingfactor);

		double diff = abs(val - real(decoded_val));
		if(diff < ERRBOUND)
			std::cout << "Ok for " << val << std::endl;
		else{
			std::cout << "Failed by " << diff << " for " << val << std::endl;
			b = val - pace;
			break;
		}
	}

	std::cout << "Working range for 2^" << scalingfactor << " as scaling factor: [" << a << ", " << b << "]" << std::endl; 

	// Validating ranges
	bool validate = true;
	cipher_t enc_val;
	cipher_init(cipher, &enc_val);	
	for(double val = a; val < b; val+=pace){
		complex<double> decoded_val;

		ckks_encrypt(cipher, &enc_val, val);
		ckks_decrypt(cipher, &decoded_val, &enc_val, sk);

		double diff = abs(val - real(decoded_val));
		if(diff < ERRBOUND)
			std::cout << "Ok for " << val << std::endl;
		else{
			std::cout << "Failed by " << diff << " for " << val << std::endl;
			validate = false;
		}
	}

	if(validate)
		std::cout << "Validated: Working range for 2^" << scalingfactor << " as scaling factor: [" << a << ", " << b << "]" << std::endl; 
	else
		std::cout << "Couldn't validate: Working range for 2^" << scalingfactor << " as scaling factor: [" << a << ", " << b << "]" << std::endl; 
	poly_free(cipher, &encoded_val);
	cipher_free(cipher, &enc_val);	
	cudaDeviceSynchronize();
	cudaCheckError();
	
	delete cipher;
	CUDAEngine::destroy();
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
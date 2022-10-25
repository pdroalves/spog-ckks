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
	sequential_add:

	The intention of this program is to demonstrate the steps required for a
	simple workload of generating a message, encrypt, and execute a lot of
	sequential additions. 
 */
int main() {
	cudaProfilerStop();

	ZZ q;

	srand(0);
	NTL::SetSeed(to_ZZ(0));

	// Params
    int nphi = 4096;
	int k = 9;
	int kl = 11;
	int64_t scalingfactor = 55;

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, scalingfactor);// Init CUDA
        
	// FV setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);
	SecretKey *sk = ckks_new_sk(cipher);
	ckks_keygen(cipher, sk);

	/////////////
	// Message //
	/////////////
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution = 
		std::uniform_real_distribution<double>(
			-sqrt(get_range(scalingfactor)/(2 * (k + 3))), sqrt(get_range(scalingfactor)/(2 * (k + 3)))
			);
	complex<double> m = {distribution(generator), distribution(generator)}, mR, *m_decrypted;

	/////////////
	// Encrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will encrypt");
	cipher_t* ct = ckks_encrypt(cipher, &m);

	//////////
	// Mul //
	////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will add");

	cipher_t *ctR = new cipher_t();
	cipher_init(cipher, ctR);
	cipher_copy(cipher, ctR, ct);
	mR = m;
	int it = 0;
	bool diff;


	poly_t *p_decrypted = new poly_t;
	poly_t *p_encoded = new poly_t;
	poly_t *p_diff = new poly_t;
	poly_init(cipher, p_decrypted);
	poly_init(cipher, p_encoded);
	poly_init(cipher, p_diff);
	ckks_decrypt_poly(cipher, p_decrypted, ct, sk);
	cipher->encodeSingle(p_encoded, &scalingfactor, m);
	poly_sub(cipher, p_diff, p_decrypted, p_encoded);
	std::cout << "Original noise: " << poly_norm_2(cipher, p_decrypted) << std::endl;
	std::cout << "Original noise diff : " << poly_norm_2(cipher, p_diff) << std::endl;

	do{
		/////////
		// Mul //
		/////////
		std::cout << "Iteration " << it << ": ";
		std::cout << (*ckks_decrypt(cipher, ct, sk)) << " + " << (*ckks_decrypt(cipher, ctR, sk)) << std::endl;
		ctR = ckks_add(cipher, ct, ctR);
		mR += m;

		ckks_decrypt_poly(cipher, p_decrypted, ctR, sk);
		cipher->encodeSingle(p_encoded, &scalingfactor, mR);
		poly_sub(cipher, p_diff, p_decrypted, p_encoded);
		std::cout << "Noise diff : " << poly_norm_2(cipher, p_diff) << std::endl;

		//////////////
		// Validate //
		/////////////
		m_decrypted = ckks_decrypt(cipher, ctR, sk);
		diff = (abs(real(*m_decrypted - mR)) + abs(imag(*m_decrypted - mR)) <= ERRBOUND);
		if(diff)
			std::cout << "We are good! (" << *m_decrypted << " - " << mR << " == " << real(*m_decrypted - mR) << ")"  << std::endl;
		else
			std::cout << "Failure /o\\ " << *m_decrypted << " != " << mR << " -- diff is " << diff << std::endl;

		it++;
	} while(diff && it < (k-1));

	/////////////
	// Release //
	/////////////
	cipher_free(cipher, ct);
	cipher_free(cipher, ctR);
	poly_free(cipher, p_decrypted);
	poly_free(cipher, p_encoded);
	poly_free(cipher, p_diff);
	delete cipher;
	CUDAEngine::destroy();

	cudaDeviceSynchronize();
	cudaCheckError();
	
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
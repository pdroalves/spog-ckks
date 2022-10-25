#include <cuPoly/settings.h>
#include <cuPoly/arithmetic/polynomial.h>
#include <SPOGCKKS/ckks.h>
#include <SPOGCKKS/ckkscontext.h>
#include <stdlib.h>
#include <NTL/ZZ.h>
#include <cuda_profiler_api.h>
#include <cxxopts.hpp>

const float ERRBOUND = 0.005;

long double get_range(int scalingfactor){
	return (long double)(CUDAEngine::RNSPrimes[0] - 1) / ((uint64_t)1 << (scalingfactor + 1));
}
/**
	toy_innerprod:

 */
int main(int argc, char *argv[]){
	/////////////////////////
	// Command line parser //
	////////////////////////
	cxxopts::Options options("importexport", "");
	options.add_options()
	("s,scalingfactor", "scalingfactor", cxxopts::value<int64_t>()->default_value("55"))
	("n,nphi", "nphi", cxxopts::value<int>()->default_value("128"))
	("k,qsize", "qsize", cxxopts::value<int>()->default_value("6"))
	("l,bsize", "bsize", cxxopts::value<int>()->default_value("8"))
	("h,help", "Print help and exit.")
	;
	auto result = options.parse(argc, argv);

	cudaProfilerStop();

	ZZ q;

	srand(0);

	// Params
    int nphi = result["nphi"].as<int>();
	int k = result["qsize"].as<int>();
	int kl = result["bsize"].as<int>();
	int64_t scalingfactor = result["scalingfactor"].as<int64_t>();

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
	Logger::getInstance()->log_info(("Scaling factor: " + std::to_string(scalingfactor) +  " - range: " + std::to_string(get_range(scalingfactor))).c_str());
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution = 
		std::uniform_real_distribution<double>(
			-sqrt(get_range(scalingfactor)/(4)), sqrt(get_range(scalingfactor)/(4))
			);

	int d = 6;	
	complex<double> m1[d], m2[d], mR = 0, *m_decrypted;
	for(int i = 0; i < d; i++){
		m1[i] = {distribution(generator), distribution(generator)};
		m2[i] = {distribution(generator), distribution(generator)};
	}

	/////////////
	// Encrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	cipher_t* ct1 = new cipher_t[d];
	cipher_t* ct2 = new cipher_t[d];
	cipher_t* ctR = new cipher_t;
	Logger::getInstance()->log_info("Will encrypt ct1");
	for(int i = 0; i < d; i++)
		ct1[i] = *ckks_encrypt(cipher, &m1[i]);
	Logger::getInstance()->log_info("Will encrypt ct2");
	for(int i = 0; i < d; i++)
		ct2[i] = *ckks_encrypt(cipher, &m2[i]);
	cipher_init(cipher, ctR);

	poly_t *p_decrypted = new poly_t;
	poly_t *p_encoded = new poly_t;
	poly_init(cipher, p_decrypted);
	poly_init(cipher, p_encoded);

	cipher_t aux;
	cipher_init(cipher, &aux);
	poly_t *p_diff = new poly_t;
	poly_init(cipher, p_diff);
	for(int i = 0; i < d; i++){
		/////////
		std::cout << "Iteration " << i << ": ";
		ckks_mul(cipher, &aux, &ct1[i], &ct2[i]);
		std::cout << (*ckks_decrypt(cipher, &ct1[i], sk)) << " * " << (*ckks_decrypt(cipher, &ct2[i], sk)) << 
		" == " << (m1[i] * m2[i]) << " =? " << (*ckks_decrypt(cipher, &aux, sk)) << std::endl;
		m_decrypted = ckks_decrypt(cipher, &aux, sk);
		double aux_diff = abs(real(*m_decrypted - (m1[i] * m2[i]))) + abs(imag(*m_decrypted - (m1[i] * m2[i])));
		if(aux_diff > ERRBOUND)
			std::cout << "Multiplication failed. Diff is too big! (" << aux_diff << ")" << std::endl;


		ckks_add(cipher, ctR, ctR, &aux);
		mR += m1[i] * m2[i];

		ckks_decrypt_poly(cipher, p_decrypted, ctR, sk);
		cipher->encodeSingle(p_encoded, &scalingfactor, mR);
		poly_sub(cipher, p_diff, p_decrypted, p_encoded);
		// std::cout << "Noise diff : " << poly_norm_2(cipher, p_diff) << std::endl;
		//////////////
		// Validate //
		/////////////
		m_decrypted = ckks_decrypt(cipher, ctR, sk);
		double diff = abs(real(*m_decrypted - mR)) + abs(imag(*m_decrypted - mR));
		if(diff <= ERRBOUND)
			std::cout << "We are good! (" << *m_decrypted << " - " << mR << " == " << real(*m_decrypted - mR) << ")" << " Err: " << diff << std::endl;
		else
			std::cout << "Failure /o\\ " << *m_decrypted << " != " << mR << " -- diff is " << diff << std::endl;
	}

	/////////////
	// Release //
	/////////////
	for(int i = 0; i < d; i++){
		cipher_free(cipher, &ct1[i]);
		cipher_free(cipher, &ct2[i]);
	}
	cipher_free(cipher, ctR);
	cipher_free(cipher, &aux);
	poly_free(cipher, p_diff);
	delete cipher;
	CUDAEngine::destroy();

	cudaDeviceSynchronize();
	cudaCheckError();
	
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
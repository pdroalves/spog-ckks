#include <cuPoly/settings.h>
#include <cuPoly/arithmetic/polynomial.h>
#include <SPOGCKKS/ckks.h>
#include <SPOGCKKS/ckkscontext.h>
#include <stdlib.h>
#include <NTL/ZZ.h>
#include <cuda_profiler_api.h>
#include <cxxopts.hpp>
#include <openssl/sha.h>

const float ERRBOUND = 0.005;

long double get_range(int scalingfactor){
	return (long double)(CUDAEngine::RNSPrimes[0] - 1) / ((uint64_t)1 << (scalingfactor + 1));
}

void print_hash2(CKKSContext *ctx, poly_t *p, int i){
	std:string residue = poly_residue_to_string(ctx, p, i);
	unsigned char obuf[20];

	SHA1((unsigned char*)residue.c_str(), strlen(residue.c_str()), obuf);
	for (i = 0; i < 20; i++) 
    	printf("%02x ", obuf[i]);
    std::cout << std::endl;
}

/**
	sequential_mul:

	The intention of this program is to demonstrate the steps required for a
	simple workload of generating a message, encrypt, and execute a lot of
	sequential multiplications. At the end of each iteration it	shall validate
	if the multiplication was successful or if it failed.
 */
int main(int argc, char *argv[]){
	/////////////////////////
	// Command line parser //
	////////////////////////
	cxxopts::Options options("sequential_mul", "");
	options.add_options()
	("n,nphi", "nphi", cxxopts::value<int>()->default_value("4096"))
	("l,qsize", "qsize", cxxopts::value<int>()->default_value("10"))
	("k,psize", "qsize", cxxopts::value<int>()->default_value("-1"))
	("h,help", "Print help and exit.")
	;
	auto result = options.parse(argc, argv);
	cudaProfilerStop();

	ZZ q;

	srand(0);
	NTL::SetSeed(to_ZZ(0));

	// Params
    int nphi = result["nphi"].as<int>();
	int k = result["qsize"].as<int>();
	// int nphi = 4096;
	// int k = 40;
	int kl = (result["psize"].as<int>() == -1? (k+1) : result["psize"].as<int>());
	int64_t scalingfactor = 55;

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, scalingfactor);// Init CUDA
        
	// FV setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);
	SecretKey *sk = ckks_new_sk(cipher);
	// sk->s.status = HOSTSTATE;
	// int s[nphi] = {0};
	// for(int i = 0; i < nphi; i++)
	// poly_set_coeff(cipher, &sk->s, i, to_ZZ(s[i]));
	// poly_copy_to_device(cipher, &sk->s);
	ckks_keygen(cipher, sk);


	// GaussianInteger evkb[CUDAEngine::get_n_residues(QBBase) * CUDAEngine::N] = {}}; 
	// GaussianInteger evka[CUDAEngine::get_n_residues(QBBase) * CUDAEngine::N] = {};

	// cudaMemcpyAsync(cipher->evk->b.d_coefs, &evkb, poly_get_residues_size(QBBase), cudaMemcpyHostToDevice, cipher->get_stream());
	// cudaCheckError();
	// cudaMemcpyAsync(cipher->evk->a.d_coefs, &evka, poly_get_residues_size(QBBase), cudaMemcpyHostToDevice, cipher->get_stream());
	// cudaCheckError();

	// // Remove the DGT layer
	// DGTEngine::execute_dgt(
	// cipher->evk->b.d_coefs,
	// QBBase,
	// FORWARD,
	// cipher
	// );
	// DGTEngine::execute_dgt(
	// cipher->evk->a.d_coefs,
	// QBBase,
	// FORWARD,
	// cipher
	// );
	
  	std::cout << "uint64_t sk[context.N * context.L] = {"  << std::endl;;
  	for(int i = 0; i < (k+kl); i++)
  		std::cout << poly_residue_to_string(cipher, &sk->s, i);
  	std::cout << std::endl;
  	std::cout << "uint64_t evkb[context.N * (context.L + context.K)] = {" << std::endl;;
  	for(int i = 0; i < (k+kl); i++)
  		std::cout << poly_residue_to_string(cipher, &cipher->evk->b, i);
  	std::cout << std::endl;
  	std::cout << "uint64_t evka[context.N * (context.L + context.K)] = {" << std::endl;;
  	for(int i = 0; i < (k+kl); i++)
  		std::cout << poly_residue_to_string(cipher, &cipher->evk->a, i);
  	std::cout << std::endl;


	/////////////
	// Message //
	/////////////
	std::default_random_engine generator;
	std::uniform_real_distribution<double> distribution = 
		std::uniform_real_distribution<double>(
			-1,1
			);
	int slots = CUDAEngine::N;
	complex<double> *m = new complex<double>[slots];
	complex<double> *mR = new complex<double>[slots];
	complex<double> *m_decrypted = new complex<double>[slots];

	for(int i = 0; i < slots; i++){
		m[i] = {distribution(generator), distribution(generator)};
		// m[i] = {1, 0};
		mR[i] = m[i];
	}

	/////////////
	// Encrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will encrypt");
	cipher_t *ct1 = ckks_encrypt(cipher, m, slots);
	cipher_t *ct2 = ckks_encrypt(cipher, mR, slots);
	
	// cipher_t *ct1, *ct2;
	// ct1 = new cipher_t;
	// ct2 = new cipher_t;
	// cipher_init(cipher, ct1);
	// cipher_init(cipher, ct2);

	// GaussianInteger ct10[CUDAEngine::get_n_residues(QBBase) * CUDAEngine::N] = {};
	// GaussianInteger ct11[CUDAEngine::get_n_residues(QBBase) * CUDAEngine::N] = {};

	// cudaMemcpyAsync(ct1->c[0].d_coefs, &ct10, poly_get_residues_size(QBase), cudaMemcpyHostToDevice, cipher->get_stream());
	// cudaCheckError();
	// cudaMemcpyAsync(ct1->c[1].d_coefs, &ct11, poly_get_residues_size(QBase), cudaMemcpyHostToDevice, cipher->get_stream());
	// cudaCheckError();

	// // Remove the DGT layer
	// DGTEngine::execute_dgt(
	// ct1->c[0].d_coefs,
	// QBase,
	// FORWARD,
	// cipher
	// );
	// DGTEngine::execute_dgt(
	// ct1->c[1].d_coefs,
	// QBase,
	// FORWARD,
	// cipher
	// );
	
	// cipher_copy(cipher, ct2, ct1);

  	std::cout << "uint64_t c10[context.N * context.L] = {";
  	for(int i = 0; i < k; i++)
  		std::cout << poly_residue_to_string(cipher, &ct1->c[0], i);
  	std::cout << "};" << std::endl;
  	std::cout << "uint64_t c11[context.N * context.L] = {";
  	for(int i = 0; i < k; i++)
  		std::cout << poly_residue_to_string(cipher, &ct1->c[1], i);
  	std::cout << "};" << std::endl;

  	std::cout << "uint64_t c20[context.N * context.L] = {";
  	for(int i = 0; i < k; i++)
  		std::cout << poly_residue_to_string(cipher, &ct2->c[0], i);
  	std::cout << "};" << std::endl;
  	std::cout << "uint64_t c21[context.N * context.L] = {";
  	for(int i = 0; i < k; i++)
  		std::cout << poly_residue_to_string(cipher, &ct2->c[1], i);
  	std::cout << "};" << std::endl;
	//////////
	// Mul //
	////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will mul");

	for(int i = 0; i < 1; i++){
	// for(int i = 0; i < k - 1; i++){
		/////////
		// Mul //
		/////////
		// std::cout << "Iteration " << (i + 1) << ": ";
		// std::cout << (*ckks_decrypt(cipher, ct1, sk)) << " * " << (*ckks_decrypt(cipher, ct2, sk)) << std::endl;
		ckks_mul_without_rescale(cipher, ct2, ct1, ct2);

		for(int j = 0; j < slots; j++)
			mR[j] *= m[j];
		//////////////
		// Validate //
		/////////////
		complex<double> *m_decrypted = ckks_decrypt(cipher, ct2, sk);

		std::cout << "ct2[0]_0 without rescale: ";
		print_hash2(cipher, &ct2->c[0], 0);
		std::cout << "ct2[1]_0 without rescale: ";
		print_hash2(cipher, &ct2->c[1], 0);

    	ckks_rescale(cipher, ct2);
    	std::cout << "ct2[0]_0 with rescale: ";
    	print_hash2(cipher, &ct2->c[0], 0);
    	std::cout << "ct2[1]_0 with rescale: ";
    	print_hash2(cipher, &ct2->c[1], 0);


		// for(int j = 0; j < slots; j++)
		int j = 0;
			std::cout << i << ") We got " << (m_decrypted[j]) << " and expected " << mR[j] << std::endl;

	}

	/////////////
	// Release //
	/////////////
	cipher_free(cipher, ct1);
	cipher_free(cipher, ct2);
	delete cipher;
	CUDAEngine::destroy();

	cudaDeviceSynchronize();
	cudaCheckError();
	
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}

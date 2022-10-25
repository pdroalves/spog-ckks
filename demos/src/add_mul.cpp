#include <cuPoly/settings.h>
#include <cuPoly/arithmetic/polynomial.h>
#include <SPOGCKKS/ckks.h>
#include <stdlib.h>
#include <NTL/ZZ.h>
#include <cuda_profiler_api.h>
#include <sstream>
#include <string>

NTL_CLIENT
const float ERRBOUND = 0.05;


std::string zToString(const ZZ &z) {
    std::stringstream buffer;
    buffer << z;
    return buffer.str();
}

std::string stringifyJson(json d){
	StringBuffer buffer;
	buffer.Clear();

	Writer<StringBuffer> writer(buffer);
	d.Accept(writer);

	return buffer.GetString();
}
/**
	add_mul:

	The intention of this program is to demonstrate the steps required for a
	simple workload of generating messages m1, m2, and m3; encrypt; execute 
	m1 * m2 + m3; and verify the result.
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

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info( ("nphi: " + std::to_string(nphi)).c_str());
	Logger::getInstance()->log_info( ("k: " + std::to_string(k)).c_str());
	Logger::getInstance()->log_info( ("kl: " + std::to_string(kl)).c_str());
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, -1);// Init CUDA
	Logger::getInstance()->log_info( ("q: " + zToString(CUDAEngine::RNSProduct) + " (" + std::to_string(NTL::NumBits(CUDAEngine::RNSProduct)) + " bits) ").c_str());
	Logger::getInstance()->log_info("==========================");

	// FV setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);
	SecretKey *sk = ckks_new_sk(cipher);
	ckks_keygen(cipher, sk);
	// Logger::getInstance()->log_debug(("Keys:" + stringifyJson(cipher->export_keys())).c_str());

	/////////////
	// Message //
	/////////////
	std::uniform_real_distribution<double> distribution = std::uniform_real_distribution<double>(0, 10);
	std::default_random_engine generator;
	complex<double> m1 = {distribution(generator), distribution(generator)};
	complex<double> m2 = {distribution(generator), distribution(generator)};
	complex<double> m3 = {distribution(generator), distribution(generator)};
	
	// Print
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info(( "m1: " + std::to_string(real(m1)) + ", " + std::to_string(imag(m1))).c_str());
	Logger::getInstance()->log_info(( "m2: " + std::to_string(real(m2)) + ", " + std::to_string(imag(m2))).c_str());
	Logger::getInstance()->log_info(( "m3: " + std::to_string(real(m3)) + ", " + std::to_string(imag(m3))).c_str());

	/////////////
	// Encrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will encrypt");
	cipher_t* ct1 = ckks_encrypt(cipher, &m1);
	Logger::getInstance()->log_debug( ("ct1: \n" + cipher_to_string(cipher, ct1)).c_str());
	cipher_t* ct2 = ckks_encrypt(cipher, &m2);
	Logger::getInstance()->log_debug( ("ct2: \n" + cipher_to_string(cipher, ct2)).c_str());
	cipher_t* ct3 = ckks_encrypt(cipher, &m3);
	
	//////////
	// Mul //
	////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will mul: m1 * m2");
	
	// Encrypted mul
	cipher_t *ctR1 = ckks_mul(cipher, ct1, ct2);
	Logger::getInstance()->log_debug( ("ctR1: \n" + cipher_to_string(cipher, ctR1)).c_str());


	// Plaintext mul
	complex<double> mR1 = m1 * m2;

	//////////
	// Add //
	////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will add: m1 * m2 + m3");
	
	// Encrypted add
	cipher_t* ctR2 = ckks_add(cipher, ctR1, ct3);
	Logger::getInstance()->log_debug( ("ctR2: \n" + cipher_to_string(cipher, ctR2)).c_str());
	
	// Plaintext add
	complex<double> mR2 = mR1 + m3;

	/////////////
	// Decrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will decrypt");
	complex<double> *m_decrypted = ckks_decrypt(cipher, ctR2, sk);


	//////////////
	// Validate //
	/////////////
	Logger::getInstance()->log_info(( "m_expected: " + 
		std::to_string(real(mR2)) + ", " + std::to_string(imag(mR2))).c_str());
	Logger::getInstance()->log_info(( "m_decrypted: " + 
		std::to_string(real(*m_decrypted)) + ", " + std::to_string(imag(*m_decrypted))).c_str());
	Logger::getInstance()->log_info(( 
		(
			real(*m_decrypted - mR2) <= ERRBOUND &&
			imag(*m_decrypted - mR2) <= ERRBOUND
		)? "Success!" : "Failure =("));

	Logger::getInstance()->log_info("\n\nDone!");

	cudaDeviceSynchronize();
	cudaCheckError();
	
	cipher_free(cipher, ct1);
	cipher_free(cipher, ct2);
	cipher_free(cipher, ct3);
	cipher_free(cipher, ctR1);
	cipher_free(cipher, ctR2);
	delete cipher;
	CUDAEngine::destroy();
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
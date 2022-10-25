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

int main() {
  cudaProfilerStop();

  ZZ q;

  srand(0);
  NTL::SetSeed(to_ZZ(0));

  // Params
  int nphi = 4096;
  int k = 2;
  int kl = 3;
  int scalingfactor = 45;

  // Set logger
  Logger::getInstance()->set_mode(INFO);

  // Init
  std::cout << "Scaling factor: " << scalingfactor << std::endl;
  CUDAEngine::init(k, kl, nphi, scalingfactor);// Init CUDA
        
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
  double m1 = -5;
  double m2 = -1;

  /////////////
  // Encrypt //
  /////////////
  Logger::getInstance()->log_info("==========================");

  for(int i = 0; i < 100; i++){

    cipher_t* ct1 = ckks_encrypt(cipher, m1);
    cipher_t *ct2 = ckks_mul_const(cipher, ct1, m2);
    complex<double> m3 = *ckks_decrypt(cipher, ct2, sk);
    std::cout << m1 << " * " << m2 << " == " << m3 << ": ";
    if(abs(real(m3) - m1*m2) > 0.01 || abs(imag(m3)) > 0.01){
      std::cout << "FAIL at index " << i << std::endl;
      exit(1);
    }
    else
      std::cout << "Correct!" << std::endl;
    
    cipher_free(cipher, ct1);
    cipher_free(cipher, ct2);
  }
  
  delete cipher;
  CUDAEngine::destroy();
  cudaDeviceReset();
  cudaCheckError();
  return 0;
}
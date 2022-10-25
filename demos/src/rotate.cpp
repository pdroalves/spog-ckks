/**
 * SPOG
 * Copyright (C) 2017-2019 SPOG Authors
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cuPoly/settings.h>
#include <cuPoly/arithmetic/polynomial.h>
#include <SPOGCKKS/ckks.h>
#include <stdlib.h>
#include <NTL/ZZ.h>
#include <cuda_profiler_api.h>
#include <complex>
#include <sstream>
#include <string>

std::string zToString(const ZZ &z) {
    std::stringstream buffer;
    buffer << z;
    return buffer.str();
}

/**
  rorate:

  The intention of this program is to demonstrate the steps required for a
  slots rotation
 */
int main() {
  cudaProfilerStop();

  ZZ q;

  srand(0);
  NTL::SetSeed(to_ZZ(0));

  // Params
  int nphi = 128;
  int k = 2;
  int kl = 3;

  // Set logger
  Logger::getInstance()->set_mode(INFO);

  // Init
  CUDAEngine::init(k, kl, nphi, 55);// Init CUDA
        
  // FV setup
  CKKSContext *cipher = new CKKSContext();
  Sampler::init(cipher);
  SecretKey *sk = ckks_new_sk(cipher);
  ckks_keygen(cipher, sk);

  /////////////
  // Message //
  /////////////
  int slots = 16;
  complex<double> *val = new complex<double>[slots];
  for(int i = 0; i < slots; i++)
    val[i] = {1, 1};
  val[0] = {2,-1};
  val[slots-1] = {-1,2};

  for(int i = 0; i < slots; i++)
    std::cout << val[i] << ", ";
  std::cout << std::endl;

  /////////////
  // Encrypt //
  /////////////
  Logger::getInstance()->log_info("==========================");
  Logger::getInstance()->log_info("Will encrypt");
  cipher_t* ct = ckks_encrypt(cipher, val, slots);
  
  cipher_t *ct1 = new cipher_t;
  cipher_t *ct2 = new cipher_t;
  cipher_init(cipher, ct1);
  cipher_init(cipher, ct2);
  cipher_copy(cipher, ct1, ct);
  /////////////
  // Rotate //
  /////////////
  for(int l = 1; l <= slots; l++){
    Logger::getInstance()->log_info("==========================");
    Logger::getInstance()->log_info("Rotation");

    ckks_rotate(cipher, ct2, ct1, 1);

    //////////////
    // Decrypt  //
    //////////////
    complex<double>* val2 = ckks_decrypt(cipher, ct2, sk);

    //////////////
    // Print  //
    //////////////
    for(int i = 0; i < slots; i++)
      std::cout << val2[i] << ", ";
    std::cout << std::endl;
    free(val2);

    std::swap(ct1, ct2);
  }

  Logger::getInstance()->log_info("==========================");
  Logger::getInstance()->log_info("Will discard higher slots");
  cipher_t *sum = ckks_sumslots(cipher, ct);
  ckks_discard_higher_slots(cipher, sum);
  complex<double> *summation = ckks_decrypt(cipher, sum, sk);
  for(int i = 0; i < sum->slots; i++)
    std::cout << summation[i] << ", ";
  std::cout << std::endl;

  cudaDeviceSynchronize();
  cudaCheckError();
  
  cipher_free(cipher, ct);
  cipher_free(cipher, ct1);
  cipher_free(cipher, ct2);
  cipher_free(cipher, sum);
  delete cipher;
  CUDAEngine::destroy();
  cudaDeviceReset();
  cudaCheckError();
  return 0;
}
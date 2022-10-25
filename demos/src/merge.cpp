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
        
  // CKKS setup
  CKKSContext *cipher = new CKKSContext();
  Sampler::init(cipher);
  SecretKey *sk = ckks_new_sk(cipher);
  ckks_keygen(cipher, sk);
  cipher->sk = sk;
  /////////////
  // Message //
  /////////////
  int slots = 16;
  std::vector<complex<double>> val(slots);
  for(int i = 0; i < slots; i++)
    val[i] = i;

  for(int i = 0; i < slots; i++)
    std::cout << val[i] << ", ";
  std::cout << std::endl;

  /////////////
  // Encrypt //
  /////////////
  Logger::getInstance()->log_info("==========================");
  Logger::getInstance()->log_info("Will encrypt");
  std::vector<cipher_t> cts;
  
  for(int l = 0; l < slots; l++)
    cts.push_back(*ckks_encrypt(cipher, &val[l], 1, 15));

  cudaDeviceSynchronize();
  Logger::getInstance()->log_info("Checking:");
  for(int l = 0; l < cts.size(); l++){
    std::cout << "Ciphertext " << l << " (" << cts[l].slots << " slots) : ";
    complex<double> *vals = ckks_decrypt(cipher, &cts[l], sk);
    std::cout << real(vals[0]) << ", " << std::endl;
    free(vals);
  }

  cudaDeviceSynchronize();

  Logger::getInstance()->log_info("Merging...");
  cipher_t *ct_merged = new cipher_t;
  ckks_merge(cipher, ct_merged, &cts[0], slots);
  Logger::getInstance()->log_info(
    (std::string("Merged ") +
        std::to_string(ct_merged->slots) +
        std::string(" slots")).c_str()
    );

  Logger::getInstance()->log_info("Checking:");
  std::cout << "Ciphertext 0: ";
  complex<double> *vals = ckks_decrypt(cipher, ct_merged, sk);
  for(int i = 0; i < ct_merged->slots; i++)
    std::cout << real(vals[i]) << ", " ;
  std::cout << std::endl;

  cudaDeviceSynchronize();
  cudaCheckError();
  
  for(int l = 0; l < slots; l++)
    cipher_free(cipher, &cts[l]);
  cipher_free(cipher, ct_merged);
  delete cipher;
  CUDAEngine::destroy();
  cudaDeviceReset();
  cudaCheckError();
  return 0;
}
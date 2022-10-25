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
	encrypt_decrypt:

	The intention of this program is to demonstrate the steps required for a
	simple workload of generating a message, encrypt, and decrypt.
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
	complex<double> val = 42.3;

	/////////////
	// Encrypt //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will encrypt");
	cipher_t* ct = ckks_encrypt(cipher, &val);

	/////////////
	// Decrypt //
	/////////////
	complex<double> *val_decrypted = ckks_decrypt(cipher, ct, sk);

	//////////////
	// Validate //
	/////////////
	std::ostringstream oss;
	oss << "val: " << val << std::endl;
	oss << "val_decrypted: " << *val_decrypted << std::endl;
	oss << "diff: " << (val - *val_decrypted) << std::endl;
	Logger::getInstance()->log_info( oss.str().c_str());

	cudaDeviceSynchronize();
	cudaCheckError();
	
	cipher_free(cipher, ct);
	delete cipher;
	CUDAEngine::destroy();
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
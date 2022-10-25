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
	int64_t scalingfactor = 55;

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, 55);// Init CUDA
        
	// FV setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);

	/////////////
	// Message //
	/////////////
	complex<double> val = 42.3, decoded_val;
	poly_t encoded_val;
	poly_init(cipher, &encoded_val);

	/////////////
	// Encode //
	/////////////
	Logger::getInstance()->log_info("==========================");
	Logger::getInstance()->log_info("Will encode");
	cipher->encodeSingle(&encoded_val, &scalingfactor, val);

	/////////////
	// Decrypt //
	/////////////
	cipher->decodeSingle(&decoded_val, &encoded_val, scalingfactor);

	//////////////
	// Validate //
	/////////////
	std::ostringstream oss;
	oss << "val: " << val << std::endl;
	oss << "decoded_val: " << decoded_val << std::endl;
	oss << "diff: " << (val - decoded_val) << std::endl;
	
	Logger::getInstance()->log_info(oss.str().c_str());

	poly_free(cipher, &encoded_val);
	cudaDeviceSynchronize();
	cudaCheckError();
	
	delete cipher;
	CUDAEngine::destroy();
	cudaDeviceReset();
	cudaCheckError();
	return 0;
}
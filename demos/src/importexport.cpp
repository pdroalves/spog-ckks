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
#include <cxxopts.hpp>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/writer.h>

std::string zToString(const ZZ &z) {
    std::stringstream buffer;
    buffer << z;
    return buffer.str();
}

/**
	importexport:

 */
int main(int argc, char *argv[]) {
	/////////////////////////
	// Command line parser //
	////////////////////////
	cxxopts::Options options("importexport", "");
	options.add_options()
	("r,read", "read keys from keys.json")
	("h,help", "Print help and exit.")
	;
	auto result = options.parse(argc, argv);

	// help
	if (result.count("help")) {
	cout << options.help({""}) << std::endl;
	exit(0);
	}

	bool read = false;
	if (result.count("read"))
		read = true;


	srand(0);
	NTL::SetSeed(to_ZZ(0));

	// Params
    int nphi = 4096;
	int k = 2;
	int kl = 3;

	// Set logger
	Logger::getInstance()->set_mode(INFO);

	// Init
	CUDAEngine::init(k, kl, nphi, 55);// Init CUDA
        
	// CKKS setup
	CKKSContext *cipher = new CKKSContext();
	Sampler::init(cipher);
	SecretKey *sk;

	// Export or import keys
	if(read){
		FILE* fp = fopen("keys.json", "r"); // non-Windows use "r"
		char readBuffer[65536];
		FileReadStream is(fp, readBuffer, sizeof(readBuffer));
		 
		json d;
		d.ParseStream(is);
		 
		fclose(fp);

		cipher->load_keys(d);
		sk = &cipher->sk;
	}else{
		sk = ckks_new_sk(cipher);
		ckks_keygen(cipher, sk);
		cipher->sk = *sk;

		json jkeys = cipher->export_keys();
		
		FILE* fp = fopen("keys.json", "wb"); // non-Windows use "w"

		char writeBuffer[65536];
		FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
		 
		Writer<FileWriteStream> writer(os);
		jkeys.Accept(writer);
		 
		fclose(fp);
	}
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
# SPOG-CKKS - Secure Processing On GPGPUs

[University of Campinas](http://www.unicamp.br), [Institute of Computing](http://www.ic.unicamp.br), Brazil.

Laboratory of Security and Cryptography - [LASCA](http://www.lasca.ic.unicamp.br),<br>
Multidisciplinary High Performance Computing Laboratory - [LMCAD](http://www.lmcad.ic.unicamp.br). <br>

Author: [Pedro G. M. R. Alves](http://www.iampedro.com), PhD. candidate @ IC-UNICAMP,<br/>

## About

SPOG-CKKS is a proof of concept for our work looking for efficient techniques to implement RLWE-based HE cryptosystems on GPUs. It implements the CKKS cryptosystem on top of [cuPoly](https://github.com/spog-library/cuPoly).


## Goal

SPOG-CKKS is an ongoing project and we hope to increase its performance and security in the course of time. Our focus is to provide:

- Exceptional performance on modern GPGPUs.
- An easy-to-use high-level API.
- Easily maintainable code. Easy to fix bugs and easy to scale.
- A model for implementations of cryptographic schemes based on RLWE.


## Disclaimer 

This is not a work targetting use in production. Thus, non-standard cryptographic implementation decisions may have been taken to simulate a certain behavior that would be expected in a production-oriented library. For instance, we mention using a truncated Gaussian distribution, built over cuRAND's Gaussian distribution, instead of a truly discrete gaussian. The security of this decision still needs to be asserted.

SPOG-CKKS is at most alpha-quality software. Implementations may not be correct or secure. Moreover, it was not tested with BFV parameters different from those in the test file. Use at your own risk.

## Installing

Note that `stable` is generally a work in progress, and you probably want to use a [tagged release version](https://github.com/spog-library/spogckks/releases).

### Dependencies
SPOG-CKKS was tested in a GNU/Linux environment with the following packages:

| Package | Version |
| ------ | ------ |
| g++ | 8.4.0 |
| CUDA | 11.0 |
| cmake | 3.13.3 |
| cuPoly | v0.3.4 |
| googletest | v1.10.0 |
| [rapidjson](https://github.com/Tencent/rapidjson) | v1.1.0 | 
| [NTL](https://www.shoup.net/ntl/) | 11.3.2 |
| [gmp](https://gmplib.org/) | 6.1.2 |

### Procedure

1) Download and install [cuPoly](https://github.com/pdroalves/cuPoly). Be careful to choose a branch compatible with the one you intend to use on SPOG (FFT or DGT).
2) Download and unzip the most recent commit of SPOG (we are not ready to start tagging releases, so the most recent commit for each branch should be taken as "the best you can get today").
2) Create spog/build.
3) Change to spog/build and run 
```
$ cmake ..
$ make
```

cmake will verify the environment to assert that all required packages are installed.

### Tests and benchmarks

SPOG contains binaries for testing and benchmarking. For testing, use spog_test. Since it is built over googletest you may apply their filters to select tests of interest. For instance,

```
./spog_test --gtest_filter=FVInstantiation/TestFV.Mul/4096*
```

runs all tests for homomorphic multiplication on a cyclotomic polynomial ring of degree 4096.

## How to use?

SPOG provides a high-level API, avoiding the need to the programmer to interact with the GPGPU. SPOG requires an initial setup to define the parameters used by FV and nothing else. [cuPoly](https://github.com/pdroalves/cupoly). is a complementary library that provides all the required arithmetic.

 ```c++
	// Params
	int nphi = 4096;
	int k = 26;
	int t = 256;
```
All CUDA calls are made and handled by [CUDAEngine](https://github.com/pdroalves/cuPoly/blob/master/include/cuPoly/cuda/cudaengine.h). Thus, before anything this object must be initialized using the choose parameters.

 ```c++
    CUDAEngine::init(k, nphi, t);// Init CUDA
```

The Homomorphic cryptosystem used by SPOG is [FV](https://eprint.iacr.org/2012/144). Hence, we use polynomial arithmetic. Such operations are provided by cuPoly in the form of [poly_t struct](https://github.com/pdroalves/cuPoly/blob/master/include/cuPoly/arithmetic/polynomial.h).   
    
 ```c++
	/////////////
	// Message //
	/////////////
	poly_t m1, m2, m3;
	poly_set_coeff(&m1, 0, to_ZZ(1));	
	poly_set_coeff(&m1, 1, to_ZZ(1));
	poly_set_coeff(&m2, 1, to_ZZ(1));
	poly_set_coeff(&m3, 1, to_ZZ(42));
```

All FV's operations are contained in class FV.  Once CUDAEngine is on, we can instantiate a FV object and generate a key set. 

 ```c++
	// FV setup
	FV *cipher = new FV(q, nphi, 0);
    q = CUDAEngine::RNSProduct;
  	cipher->keygen();
```
  
Encryption, decryption, and homomorphic operations are executed by FV's methods.

 ```c++
	/////////////
	// Encrypt //
	/////////////
	cipher_t* ct1 = cipher->encrypt(m1);
	cipher_t* ct2 = cipher->encrypt(m2);
	cipher_t* ct3 = cipher->encrypt(m3);

    //////////
	// Mul //
	////////
	cipher_t *ctR1 = cipher->mul(*ct1, *ct2);

	//////////
	// Mul //
	////////	
	cipher_t* ctR2 = cipher->add(*ctR1, *ct3);
  
	/////////////
	// Decrypt //
	/////////////
	poly_t *m_decrypted = cipher->decrypt(*ctR2);
```

Once we are done, CUDAEngine must be destroyed and the objects created must be released.

 ```c++
    poly_free(&m1);
    poly_free(&m2);
    poly_free(m_decrypted);
    delete cipher;
    cipher_free(ct1);
    cipher_free(ct2);
    cipher_free(ct3);
    cipher_free(ctR1);
    cipher_free(ctR2);
    CUDAEngine::destroy();
```

## Citing

## Disclaimer
SPOG is at most alpha-quality software. Implementations may not be correct or secure. Moreover, it was not tested with FV parameters different from those in the test file. Use at your own risk.

## Licensing

SPOG is released under GPLv3.

**Privacy Warning:** This site tracks visitor information.

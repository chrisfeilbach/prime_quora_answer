// Written by Chris Feilbach (C) 2019 for a Quora answer
// All rights reserved.

#include <iostream>
#include <cstring>
#include <sys/mman.h>

int main(int argc, char** argv) {
    
    // NOTE: Capped at 20,000,000 for reasonable runtime.
    const uint32_t max = 1000000000;
    //uint32_t prime_count = 0;

    // Implement the Sieve of Eratosthenes.
    uint8_t *sieve = (uint8_t*)mmap(NULL, (max/sizeof(uint8_t))+1,
		         PROT_READ | PROT_WRITE,
			 MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    uint32_t sieve_size = (max/sizeof(uint8_t)) + 1;
    if (sieve == (void*)-1) {
        perror("Unable to mmap(): ");
	return 1;
    }

    // Clear out the sieve.
    memset(sieve, 0, sieve_size);

    munmap(sieve, (max/sizeof(uint8_t))+1);

    return 0;
}

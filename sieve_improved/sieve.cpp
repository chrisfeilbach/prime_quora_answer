#include <iostream>
#include <ctype.h>
#include <sys/mman.h>
#include <sys/time.h>

#define MIN_PRIME 2
#define MAX_PRIME 2000000000

int main(int argc, char** argv) {

    // Calculate sieve size and allocate.
    //
    constexpr const unsigned int sieve_size = (MAX_PRIME - MIN_PRIME) / 8;

    uint64_t* sieve = (uint64_t*)mmap(nullptr, sieve_size, PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    std::cout << "sizeof(int): " << sizeof(int) << std::endl;
    if (sieve == MAP_FAILED) {
        perror("Unable to mmap() sieve: ");
    }

    // Perform a really simple sieve and count the number of primes along 
    // the way.
    //
    uint64_t prime_count  = 0;
    uint64_t sieve_data   = 0;
    uint64_t read_counter = MIN_PRIME;
    uint64_t read_pointer = 0;
    for(uint64_t i=MIN_PRIME;i*i<=MAX_PRIME;i++) {
        // There are 64 bits in a uint64_t, so only read one every
	// 64 iterations through the loop. Start that count here.
	//
	++read_counter;

        uint64_t bit_offset = i & 0x3Full;
	uint64_t mask       = 1ull << bit_offset;
	uint64_t sieve_bit  = sieve_data & mask;

	if (sieve_bit == 0) {
            // Found a prime, mark all the composites.
	    //
	    //std::cout << i << std::endl;
	    for(uint64_t j=i*i;j<=MAX_PRIME;j+=i) {
                // Simple algorithm here, do load and store for every
		// composite.
		//
		uint64_t address    = j >> 6;
		uint64_t bit_offset = j & 0x3Full;
		uint64_t mask       = 1ull << bit_offset;
		uint64_t data       = sieve[address] & mask;
		if (!data) {
                    sieve[address] |= mask;
		}
	    }
	    // Reload sieve_data for the loop below, because it may
	    // have been updated upon setting composites.
	    sieve_data = sieve[read_pointer];
	}

	// Determine whether to load the next 64-bits.
	//
	if (read_counter == 64) {
            read_pointer++;
	    read_counter = 0;
	    sieve_data = sieve[read_pointer];
	}
    }

    for(uint64_t i=MIN_PRIME;i<MAX_PRIME;i++) {
        prime_count += (sieve[i >> 6] & (1ull << (i & 0x3Full))) ? 0 : 1;
    }

    // For sanity, print out prime count.
    //
    std::cout << prime_count << std::endl;

    munmap(sieve, sieve_size);
    return 0;
}

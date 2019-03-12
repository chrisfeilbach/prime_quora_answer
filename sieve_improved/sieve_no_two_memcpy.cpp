#include <iostream>
#include <vector>
#include <ctype.h>
#include <cmath>
#include <sys/mman.h>
#include <time.h>

// Multiples of two are optimized out of the sieve.
#define MIN_PRIME 3
#define START_PRIME 13
#define MEMCPY_OPT  29
#define MAX_PRIME 2000000000

void sieve_preload(uint64_t* sieve);
bool check_prime(uint64_t prime);

int main(int argc, char** argv) {

    // Calculate sieve size and allocate.
    //
    constexpr const unsigned int sieve_size = (MAX_PRIME - MIN_PRIME) / 8 / 2;

    uint64_t* sieve = (uint64_t*)mmap(nullptr, 
                                      sieve_size + MEMCPY_OPT*sizeof(uint64_t), 
                                      PROT_READ | PROT_WRITE,
                                      MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);

    //std::cout << "sizeof(int): " << sizeof(int) << std::endl;
    if (sieve == MAP_FAILED) {
        perror("Unable to mmap() sieve: ");
    }

    // Collect some perf stats.
    uint64_t capacity = sqrt(MAX_PRIME);
    std::vector<long> time_per_iteration(capacity);
    

    // Perform a really simple sieve and count the number of primes along 
    // the way.
    //
    uint64_t prime_count  = 0;
    uint64_t sieve_data   = 0;
    //uint64_t read_counter = MIN_PRIME;
    uint64_t read_pointer = 0;
    //sieve_preload(sieve);
    // No even numbers, so increment by two.
    //
    for(uint64_t i=MIN_PRIME;i*i<=MAX_PRIME;i+=2) {
        //struct timespec start, end;
        //if (clock_gettime(CLOCK_MONOTONIC_RAW, &start)) {
        //    std::cerr << "Error getting time." << std::endl;
        //    return EXIT_FAILURE;
        //}
        // There are 64 bits in a uint64_t, so only read one every
	// 64 iterations through the loop. Start that count here.
	//

        uint64_t bit_offset = (i >> 1) & 0x3Full;
        //std::cout << "Address = " << read_pointer << " bit_offset = " << bit_offset << std::endl;
	uint64_t mask       = 1ull << bit_offset;
	uint64_t sieve_bit  = sieve_data & mask;

	if (sieve_bit == 0) {
            if (false && !check_prime(i)) {
                std::cerr << "Composite marked not prime " << i << std::endl;
                return EXIT_FAILURE;
            }
            // Found a prime, mark all the composites.
	    //
	    //std::cout << i << std::endl;
            bool memcpy_optimization = (i <= MEMCPY_OPT);
            uint64_t j          = i*i;
            uint64_t address    = 0;
            uint64_t bit_offset = 0;
	    for(;j<=MAX_PRIME;j+=2*i) {
                // If this is an even number, skip.
                //
                //if (j % 2 == 0) continue;


                // Address now needs to shift off the LSB, plus 6.
                //
                __builtin_prefetch(&sieve[((j + 2*i) >> 1) >> 6], 0, 0);
		address             = (j >> 1) >> 6;
		bit_offset          = (j >> 1) & 0x3Full;

                // An optimization exists where a mask can be generated and
                // then bitwise replicated across the sieve. To start this
                // optimization off, break out of the loop when it is enabled
                // and once address is divisible by the prime number.
                // 
                if (false && memcpy_optimization && 
                    ((j >> 1) >> 6) % i == 0 &&
                    address != 0) {
                    break;
                }

		uint64_t mask   = 1ull << bit_offset;
                sieve[address] |= mask;
                    //std::cout << "Marking " << j << " address " << address << " bif offset " << bit_offset << std::endl;
                    //printf("sieve hex: 0x%lx\n", sieve[address]);
                    //if (j == 255) {
                    //    std::cerr << "i = " << i << " j = " << j << std::endl;
                    //    return EXIT_FAILURE;
                    //}
	    }
            if (false && memcpy_optimization) {
                // Populate a mask, and them memcpy() the thing instead of
                // populating each bit.
                uint64_t big_mask[MEMCPY_OPT];
                for(int i=0;i<MEMCPY_OPT;i++) { big_mask[i] = 0; }
		uint64_t big_mask_address    = 0;
                uint64_t big_mask_bit_offset = bit_offset;
                //std::cout << "big_mask_address = " << big_mask_address << " big_mask_bit_offset = " << big_mask_bit_offset << std::endl;
                // Populate the mask that spans n uint64_t elements.
                // This guarantees by the end of it we have a repeating 
                // pattern, allowing us to blast it out to memory.
                //
                while (big_mask_address < i) {
                    if (big_mask_address >= i) { 
                        std::cerr << "Buffer overrun i = " << i << " big_mask_address = " << big_mask_address << "." << std::endl;
                        return EXIT_FAILURE;
                    }

                    big_mask_address              = ((j >> 1) >> 6) - address;
                    big_mask_bit_offset           = (j >> 1) & 0x3Full;
                    uint64_t mask                 = 1ull << big_mask_bit_offset;
                    big_mask[big_mask_address]   |= mask;

                    j = j + 2*i;
                }
                // Memory copy.
                //
                //std::cout << "Starting at address " << address << " bit_offset" << bit_offset << std::endl;
                for(uint64_t k=address;
                    k<=(sieve_size/sizeof(uint64_t));k+=i) {
                    for (uint64_t l=0;l<i;l++) {
                        sieve[k+l] = sieve[k+l] | big_mask[l];
                    }
                }
            }
	    // Reload sieve_data for the loop below, because it may
	    // have been updated upon setting composites.
	    sieve_data = sieve[read_pointer];
	} else if (sieve_bit == 1) {
            if (false && check_prime(i)) {
                std::cerr << "Prime marked composite " << i << std::endl;
                return EXIT_FAILURE;
            }
        }

	// Determine whether to load the next 64 bits.
	//
	if (bit_offset == 63) {
            read_pointer++;
	    sieve_data = sieve[read_pointer];
	}
        //clock_gettime(CLOCK_MONOTONIC_RAW, &end);

        //time_per_iteration.push_back(end.tv_nsec - start.tv_nsec);
    }

    for(uint64_t i=MIN_PRIME;i<MAX_PRIME;i+=2) {
        uint64_t is_prime = (sieve[(i >> 1) >> 6] & (1ull << ((i >> 1) & 0x3Full))) ? 0 : 1;
        prime_count += is_prime;
        //if (is_prime) std::cout << "Prime " << i << std::endl;
    }

    //std::cout << "Performance data" << std::endl;
    //for (suseconds_t time : time_per_iteration) {
    //    std::cout << time << std::endl;
    //}
    ++prime_count;

    // For sanity, print out prime count.
    //
    std::cout << prime_count << std::endl;

    munmap(sieve, sieve_size);
    return 0;
}

void sieve_preload(uint64_t* sieve) {
    int64_t counters[4]     = {-2, -2, -1, -8};
    const int64_t values[4] = {-3, -5, -7, -11};

    // Keep track of divisibility for a few of the
    // small primes, marking them as we walk through
    // the sieve only one time.
    //
    for (uint64_t i=START_PRIME;i<=MAX_PRIME;) {
        // This is guaranteed by code below this block to
        // be odd, and divisible by one of the primes we care about.
        //
        uint64_t address    = (i >> 1) >> 6;
        uint64_t bit_offset = (i >> 1) & 0x3Full;
        uint64_t mask       = 1ull << bit_offset;
        sieve[address]     |= mask;

        // Determine the how far to step the loop based on the values.
        //
        bool was_even_step = true;
        while (was_even_step) {
            //std::cout << "i = " << i << std::endl;
            int64_t step = 13;
            for (int i=0;i<4;i++) {
                //std::cout << "Before counter[" << i << "]" << " = " << counters[i] << std::endl;
                if (-counters[i] < step) { step = -counters[i]; }
            }
            //std::cout << "step = " << step << std::endl;
            for (int i=0;i<4;i++) {
                counters[i] += step;
                if (counters[i] == 0) counters[i] = values[i];
                //std::cout << "After counter[" << i << "]" << " = " << counters[i] << std::endl;
            }
            //std::cout << "Taking step " << step << std::endl;
            i += step; 
            if ((i & 1) != 0) { was_even_step = false; }
        }
        //std::cout << "Took step." << std::endl;
    }
}

bool check_prime(uint64_t prime) {
    bool is_prime = true;
    for(uint64_t i=2;i*i<=prime;i++) {
        if (prime % i == 0) {
            is_prime = false;
            break;
        }
    }

    return is_prime;
}

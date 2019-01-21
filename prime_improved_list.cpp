// Written by Chris Feilbach (C) 2019 for a Quora answer
// All rights reserved.

#include <iostream>

int main(int argc, char** argv) {
    
    // NOTE: Capped at 20,000,000 for reasonable runtime.
    const uint32_t max = 20000000;
    uint32_t prime_count = 2;

    uint32_t list[LIST_COUNT];
    uint32_t list_count = 2;

    // Initialize the list for primes 2 and 3.
    list[0] = 2;
    list[1] = 3;

    // MINOR OPTIMIZATION: 2 billion isn't prime by inspection, don't compute it.
    // FURTHER OPTIMIZATION: Start at 5, and the only search odd numbers.
    // FURTHER OPTIMIZATION: This for loop creates a list of the first LIST_COUNT
    // prime numbers, to be used in another for loop below.
    uint32_t i = 5;
    for(;i<max;i=i+2) {
	bool prime = true;
	// OPTIMIZATION: Only search through sqrt(i) divisors.
        for(uint32_t j=2;j*j<=i;j++) {
            if (i % j == 0) {
                prime = false;
		break;
	    }
	}
	
	// MISSED OPPORTUNITY: Get rid of this if and use += to avoid branching.
	if (prime == true) ++prime_count;
	if (prime == true && list_count < LIST_COUNT) {
            list[list_count++] = i;
	    if (list_count == LIST_COUNT) break;
	}
    }

    // OPTIMIZATION: Loop over the first 1024 prime numbers and divide by
    // each. This allows us to bail out earlier when we find a composite
    // number, and reduces the number of divisions slightly for a prime
    // number.
    for(i=list[LIST_COUNT-1]+2;i<max;i=i+2) {
        bool prime = true;
	for(uint32_t j=0;j<LIST_COUNT;j++) {
            if (i % list[j] == 0) {
                prime = false;
		break;
	    }
	}

	if (prime == false) continue;

	// Search the remainder of the numbers above the end of the
	// list.
	for(uint32_t j=list[LIST_COUNT-1];j*j<=i;j++) {
            if (i % j == 0) {
                prime = false;
		break;
	    }
	}

	if (prime == true) ++prime_count;
    }

    std::cout << "Number of primes between 2 and " << max << ": " <<
        prime_count << std::endl;

    return 0;
}

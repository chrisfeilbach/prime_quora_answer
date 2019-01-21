// Written by Chris Feilbach (C) 2019 for a Quora answer
// All rights reserved.

#include <iostream>

int main(int argc, char** argv) {
    
    // NOTE: Capped at 20,000,000 for reasonable runtime.
    const uint32_t max = 20000000;
    uint32_t prime_count = 2;

    // MINOR OPTIMIZATION: 2 billion isn't prime by inspection, don't compute it.
    // FURTHER OPTIMIZATION: Start at 5, and the only search odd numbers.
    for(uint32_t i=5;i<max;i=i+2) {
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
    }

    std::cout << "Number of primes between 2 and " << max << ": " <<
        prime_count << std::endl;

    return 0;
}

#include "cor/utils/utils.hpp"

#include <random>
#include <algorithm>

std::string random_string(std::size_t length)
{
    std::string str(length, 0);

    // default chars used to generate a random string
    std::string const default_chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

    // create a non-deterministic random number generator
    std::default_random_engine rng(std::random_device{}());

    // create a random number "shaper" that will give us
    // uniformly distributed indices into the character set
    std::uniform_int_distribution<std::size_t> dist{0, default_chars.length()-1};

    // generate random string
    std::generate_n(str.begin(), length, [&]{ return default_chars[dist(rng)]; });

    return str;
}

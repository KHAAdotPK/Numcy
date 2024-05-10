#include "main.hh"

int main(int argc, char* arhv[])
{
    Collective<double> W1;

    W1 = Numcy::Random::randn(DIMENSIONS{100, 115, NULL, NULL});

    return 0;
}
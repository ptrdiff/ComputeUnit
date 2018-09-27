#include <iostream>
#include <fstream>

int main()
{
    double a[6];
    while(true)
    {
        for (double& i : a)
        {
            std::cin >> i;
        }

        for (double i : a)
        {
            std::cout << ' ' << i;
        }
        std::cout << std::flush;
    }
}
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <ctime>

//Функция генерации случайного вещественного числа
double dRand(double min, double max)
{
    double f = static_cast<double>(rand()) / RAND_MAX;
    return min + (max - min) * f;
}

int main()
{
    std::fstream file("input.txt", std::ios::out | std::ios::trunc);
    uint64_t amount;
    std::cout << "Amount: " << std::endl;
    std::cin >> amount;
    std::srand(std::time(nullptr));
    file.precision(16);
    if (!file.is_open())
        std::cout << "Error! File did not open!" << std::endl;
    else
        for (size_t i = 0; i < amount; i++) {
            file << dRand(-1000000000, 1000000000) << std::endl;
        }
    return 0;
}

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include <cmath>

int const AMOUNT_OF_NUMBERS = 1000000; //Количество чисел для сортировки
int const AMOUNT_OF_THREADS = 4; //Число потоков
int const SIZE_OF_CHUNK = 100000; //Размер блока, который считывается из файла за раз
int const PRECISION = 16; //Количество знаков после запятой при вводе/выводе

struct InformationForMerge
{
    int fileNumber;
    std::string sCurrentNumber;
    double dCurrentNumber;
};

template <typename T>
T partition(T start, T end)
{
    T borderElement = start, i = start, j;
    for (j = ++start; j != end; j++) {
        if (*j <= *borderElement) {
            i++;
            std::swap(*i, *j);
        }
    }
    std::swap(*borderElement, *i);
    return i;
}

//Функция сортировки
template <typename T>
void quickSort(T start, T end)
{
    T borderElement;
    if (start != end) {
        borderElement = partition(start, end);
        quickSort(start, borderElement);
        quickSort(++borderElement, end);
    }
}

//Функция слияния файла и массива чисел
//Нужна для создания итогового файла каждого потока
void merge(std::fstream & file, std::vector<double> const & array, size_t size, int threadNumber)
{
    size_t i = 0;
    std::string currentNumber, oldName = "outputT" + std::to_string(threadNumber) + ".txt",
                               newName = "output" + std::to_string(threadNumber) + ".txt";
    std::fstream resultFile(oldName, std::ios::out | std::ios::trunc); //Вспомогательный файл, в который будет запись
    if (!resultFile.is_open())
        throw 1;
    else {
        resultFile.precision(PRECISION);
        std::getline(file, currentNumber);
        //Цикл слияния
        while (!file.eof() || (i < size)) {
            if (file.eof()) {
                resultFile << array[i] << std::endl;
                i++;
            }
            else {
                if (i == size) {
                    resultFile << currentNumber << std::endl;
                    std::getline(file, currentNumber);
                }
                else {
                    if (std::stod(currentNumber) < array[i]) {
                        resultFile << currentNumber << std::endl;
                        std::getline(file, currentNumber);
                    }
                    else {
                        resultFile << array[i] << std::endl;
                        i++;
                    }
                }
            }
        }
        //Подмена исходного файла новым
        file.close(); //Исходный файл закрывается
        std::remove(newName.c_str()); //Исходный файл удаляется
        resultFile.close(); //Закрывается новый файл
        std::rename(oldName.c_str(), newName.c_str()); //Новый файл получает имя исходного
        file.open(newName, std::ios::in | std::ios::out); //В дескриптор исходного файла помещается новый файл
    }
}

//Функция потока
//Выполняет считывание очередного блока из файла inputFile
//Далее выполняет сортировку блока и слияние результата с файлом outputFile
void process(std::fstream & inputFile, std::fstream & outputFile, int threadNumber, int &errorFlag)
{
    std::vector<double> currentChunk(SIZE_OF_CHUNK);
    std::string currentNumber;
    int sizeCurrentChunk;
    inputFile.seekg(0);
    while (!inputFile.eof()) {
        sizeCurrentChunk = 0;
        //Считывание блока
        while ((sizeCurrentChunk < SIZE_OF_CHUNK) && std::getline(inputFile, currentNumber)) {
            currentChunk[sizeCurrentChunk] = std::stod(currentNumber);
            sizeCurrentChunk++;
        }
        //Сортировка
        quickSort(currentChunk.begin(), currentChunk.begin() + sizeCurrentChunk);
        try {
            //Слияние
            merge(outputFile, currentChunk, sizeCurrentChunk, threadNumber);
        }
        catch(int e) {
            errorFlag = 1;
            return;
        }
    }
}

//Функция для проверки окончания слияния итоговых файлов
bool endCheck(std::vector<InformationForMerge> const & inf)
{
    bool result = false;
    for (size_t i = 0; i < inf.size(); i++)
        if (!std::isinf(inf[i].dCurrentNumber)) {
            result = true;
            break;
        }
    return result;
}

//Функция для проверки наличия флага ошибки
bool errorCheck(std::vector<int> const & errorFlags)
{
    int result = 0;
    for (int x: errorFlags)
        result += x;
    return result;
}

//Функция для слияния файлов, полученных в потоках, в один итоговый файл
void merge(std::vector<std::fstream> & files, std::fstream & resultFile)
{
    std::vector<InformationForMerge> infForMerge(files.size());
    InformationForMerge currentMin;
    size_t i;
    for (i = 0; i < files.size(); i++) {
        infForMerge[i].fileNumber = i;
        std::getline(files[i], infForMerge[i].sCurrentNumber);
        if (!infForMerge[i].sCurrentNumber.empty()) {
            infForMerge[i].dCurrentNumber = std::stod(infForMerge[i].sCurrentNumber);
        }
        else
            infForMerge[i].dCurrentNumber = INFINITY;
    }
    //Пока все файлы не считаны до конца цикл будет продолжаться
    while(endCheck(infForMerge)) {
        currentMin = infForMerge[0];
        for (i = 1; i < files.size(); i++) {
            //Определение минимума из текущих чисел
            currentMin = std::min(currentMin, infForMerge[i], [](InformationForMerge const & lhs, InformationForMerge const & rhs)
                                                                        {
                                                                            return lhs.dCurrentNumber < rhs.dCurrentNumber;
                                                                        });
        }
        //Вывод минимального значения в итоговый файл
        resultFile << currentMin.sCurrentNumber << std::endl;
        //Чтение следующего числа из файла, которому соответствовало минимальной число
        std::getline(files[currentMin.fileNumber], infForMerge[currentMin.fileNumber].sCurrentNumber);
        //Проверка на достижение конца файла
        if (!infForMerge[currentMin.fileNumber].sCurrentNumber.empty()) {
            //Если нет, то число преобразуется в double
            infForMerge[currentMin.fileNumber].dCurrentNumber = std::stod(infForMerge[currentMin.fileNumber].sCurrentNumber);
        }
        else
            //Иначе обозначаем, что данный файл закончился
            infForMerge[currentMin.fileNumber].dCurrentNumber = INFINITY;
    }
}

int main()
{
    std::vector<std::thread> threadArray; //Массив дескрипторов потока
    std::vector<std::fstream> threadInputFiles, //Массив файловых дескрипторов для входных данных каждого потока
                              threadOutputFiles; //Массив файловых дескрипторов для выходных файлов каждого потока
    std::vector<int> errorFlags(AMOUNT_OF_THREADS, 0); //Массив флагов ошибки каждого потока
    int i = 0;
    std::string currentNumber;
    std::fstream inputFile("input.txt", std::ios::in | std::ios::out), //Дескриптор исходного файла с данными
                 outputFile("output.txt", std::ios::out | std::ios::trunc); //Дескриптор файла, в котором окажется результат
    //Инициализация
    try {
        if (!inputFile.is_open() || !outputFile.is_open())
            throw 1;
        for (i = 0; i < AMOUNT_OF_THREADS; i++) {
            threadInputFiles.push_back(std::fstream("input" + std::to_string(i) + ".txt",
                                       std::ios::in | std::ios::out | std::ios::trunc));
            threadOutputFiles.push_back(std::fstream("output" + std::to_string(i) + ".txt",
                                        std::ios::in | std::ios::out | std::ios::trunc));
            if (!threadInputFiles[i].is_open() || !threadOutputFiles[i].is_open())
                throw 1;
            else {
                threadInputFiles[i].precision(PRECISION);
                threadOutputFiles[i].precision(PRECISION);
            }
        }
    }
    catch (int e) {
        std::cout << "Error! File did not open!" << std::endl;
        return 0;
    }
    inputFile.precision(PRECISION);
    outputFile.precision(PRECISION);
    std::cout.precision(PRECISION);
    i = 0;
    //Заполнение файлов с исходными данными для каждого потока
    while ((i < AMOUNT_OF_NUMBERS) && (std::getline(inputFile, currentNumber))) {
        threadInputFiles[i % AMOUNT_OF_THREADS] << currentNumber << std::endl;
        i++;
    }
    //Запуск потоков
    for (i = 0; i < AMOUNT_OF_THREADS; i++) {
        threadArray.push_back(std::thread(process, std::ref(threadInputFiles[i]), std::ref(threadOutputFiles[i]), i, std::ref(errorFlags[i])));
    }
    //Ожидание завершения потоков
    for (i = 0; i < AMOUNT_OF_THREADS; i++) {
        threadArray[i].join();
    }
    std::cout << "Threads completed!" << std::endl;
    //Если ошибок нет, то происходит слияние промежуточных результатов в итоговый файл
    if (!errorCheck(errorFlags))
        merge(threadOutputFiles, outputFile);
    else
        std::cout << "Error! Sorting failed!" << std::endl;
    //Удаление файлов потоков
    for (i = 0; i < AMOUNT_OF_THREADS; i++) {
        threadInputFiles[i].close();
        std::remove(std::string("input" + std::to_string(i) + ".txt").c_str());
        threadOutputFiles[i].close();
        std::remove(std::string("output" + std::to_string(i) + ".txt").c_str());
    }
    return 0;
}

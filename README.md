# Infotecs-task
Здравствуйте! Это описание второго задания на стажировку.
Была реализована быстрая сортировка. Сложность сортировки O(n * log(n)).
Но, так как необходимо учитывать ограничения по памяти, пришлось разбивать исходные данные на блоки.
Таким образом сложонсть всего алгоритма получилась O(n * log(k)) + O(n * (n + k) / (2 * k)), где k - это постоянный размер блока.
Данные считываются из файла input.txt. Результат выводится в файл output.txt.
Файл test_generator.cpp - это генератор исходных данных. Эта программа создает файл, в котором в каждой строке содержится одно число. Программа сортировки считывает данные из файлов только такого формата.
Программа использует только стандартную библиотеку С++, поэтому для сборки требуется просто скомпилировать ее.

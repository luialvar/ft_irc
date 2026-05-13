#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>

std::vector<std::string> split(const std::string& str, char pattern) 
{
    size_t posInit = 0;
    size_t posFound;
    std::vector<std::string> results;

    while ((posFound = str.find(pattern, posInit)) != std::string::npos) {
        results.push_back(str.substr(posInit, posFound - posInit));
        posInit = posFound + 1;
    }

    results.push_back(str.substr(posInit));

    return results;
}

std::vector<std::string> splitKeys(const std::string& str)
{
    std::vector<std::string> result;
    std::string current;

    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == ',')
        {
            result.push_back(current);
            current.clear();
        }
        else
        {
            current += str[i];
        }
    }
    result.push_back(current);

    return result;
}
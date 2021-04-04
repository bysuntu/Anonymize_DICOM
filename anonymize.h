#ifndef ANONYMIZE_H
#define ANONYMIZE_H
#include<string.h>
#include<map>
#include<iostream>

std::map<std::string, int> loadCSV(const char *fileName, std::vector<std::vector<std::string> >& table);
void parseFolder(std::string inputFolder, std::string outputFolder, std::vector<std::vector<std::string> >& table, std::map<std::string, int> caseMap);
bool saveCSV(const char *fileName, std::vector<std::vector<std::string> > table);

#endif
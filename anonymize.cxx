#include <string>
//#include <sstream>
//#include <array>
#include <gdcmReader.h>
#include <gdcmWriter.h>
#include <gdcmGlobal.h>
#include <gdcmDicts.h>
#include <gdcmDict.h>
#include <gdcmAttribute.h>
#include <gdcmStringFilter.h>
#include <gdcmScanner.h>
#include <gdcmImageReader.h>
#include <gdcmImage.h>
#include <gdcmDirectory.h>
//#include <gdcmBitmapToBitmapFilter.h>
#include <gdcmOrientation.h>
#include <gdcmJPEGCodec.h>
#include <gdcmImage.h>
#include <gdcmFile.h>
#include <gdcmAnonymizer.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <windows.h>
#include <dirent.h>
#include <sstream>

#include <vector>
#include <map>
#include <set>
#include <utility>
#include "anonymize.h"

#define isFile 0x8000

template<typename T>

std::string meToString(T v){
	std::ostringstream temp_;
	temp_ << v;
	return temp_.str();
}

std::string removeSpace(std::string Str)
{
	std::string outStr;
	for (unsigned int i=0; i < Str.size(); i++)
	{
		if (!isspace(Str[i]) && Str[i]!='/' && Str[i] != '-')
		{
			outStr += Str[i];
		}
	}
	return outStr;
}

std::string replaceSpecialCharacters(std::string filepath)
{
	std::string newpath;
	char characters[] = { '\\', '/', '*', '"', '<', '>', ':', '|', '?', ' ' };
	for (unsigned int i = 0; i < filepath.size(); i++)
		for (unsigned int j = 0; j < std::strlen(characters); j++)
			if (filepath[i] == characters[j])
				filepath[i] = '_';
	return filepath;
}

std::map<std::string, int> loadCSV(const char *fileName, std::vector<std::vector<std::string> >& table)
{
	std::map<std::string, int> caseMap;
	std::cout << fileName << std::endl;
	std::ifstream file;
	file.open(fileName);
	std::string line;

	if (file.is_open())
	{
		int n = 0;
		while (std::getline(file,line))
		{
			table.push_back(std::vector<std::string>(10, ""));
			// std::cout << line << std::endl;
			for (int i = 0; i < 10; i++)
			{
				if (line.find(",") >= 0 && line.find(",") < 1000)
				{
					table[n][i] = line.substr(0, line.find(","));
					if ((i == 1 || i == 5 || i == 6) && n > 0)
						table[n][i] = removeSpace(table[n][i]);
					/*
					if (i == 9 && n > 0)
						table[n][i] = "0";
					*/
					line.erase(0, line.find(",") + 1);
					/*
					std::cout << table[n][i];
					for (int j = 0; j < (20 - (int)table[n][i].length()); j++)
						std::cout << " ";
					*/
				}
				else if (line.find("\n") > 0){
					table[n][i] = line.substr(0, line.find("\n"));
				}
				else
				{
					break;
				}
			}
			if (n > 0)
				table[n][8] = "0";

			// std::cout << std::endl;
			caseMap[table[n][1]] = n;
			n++;
		}
	}
	else
	{
		std::cout << "Cannot read" << std::endl;
	}
	return caseMap;
}

void modifyTags(std::map<const gdcm::Tag, std::string>& modiTags, std::string bbid, std::string diag){
	const gdcm::Tag tPName(0x10, 0x10);
	const gdcm::Tag tPID(0x10, 0x20);
	const gdcm::Tag tPAge(0x10, 0x1010);
	const gdcm::Tag tPDOB(0x10, 0x30);
	const gdcm::Tag tPSex(0x10, 0x40);
	const gdcm::Tag tPSize(0x10, 0x1020);
	const gdcm::Tag tPWeight(0x10, 0x1030);
	const gdcm::Tag tDescrb(0x8, 0x1030);
	const gdcm::Tag tInstitute(0x8, 0x80);
	const gdcm::Tag tAddress(0x8, 0x81);
	const gdcm::Tag tPhysician(0x8, 1050);

	modiTags[tPName] = bbid;
	modiTags[tPID] = bbid;
	modiTags[tPAge] = "";
	modiTags[tPDOB] = "";
	modiTags[tPSex] = "";
	modiTags[tPSize] = "";
	modiTags[tPWeight] = "";
	modiTags[tDescrb] = diag;
	modiTags[tInstitute] = "";
	modiTags[tAddress] = "";
	modiTags[tPhysician] = "";
}

std::pair<bool, std::string> rwGDCM(const char *fileName, const char *outputFolder, std::vector<std::vector<std::string> >& table, std::map<std::string, int> caseMap){
	gdcm::Reader reader;
	reader.SetFileName(fileName);
	if (!reader.Read())
	{
		std::cerr << "Could not read: " << fileName << std::endl;
		return std::make_pair(false, "");
	}

	gdcm::File &file = reader.GetFile();
	gdcm::DataSet &ds = file.GetDataSet();
	const gdcm::Global& g = gdcm::Global::GetInstance();
	const gdcm::Dicts &dicts = g.GetDicts();
	const gdcm::Dict &pubdict = dicts.GetPublicDict();

	gdcm::StringFilter sf;
	sf.SetFile(file);

	const gdcm::Tag tStudyProtocol(0x18, 0x1030);
	const gdcm::Tag tPID(0x10, 0x20);
	const gdcm::Tag tSeriesNum(0x20, 0x11);
	gdcm::Tag tSOPUID;
	pubdict.GetDictEntryByName("SOP Instance UID", tSOPUID);

	std::string studyProtocol = sf.ToString(tStudyProtocol);
	if (removeSpace(studyProtocol).size()==0)
	{
		std::cout << "Remove" << std::endl;
		std::cout << fileName << std::endl;
		std::cout << sf.ToString(tPID) << std::endl;
		return std::make_pair(false, "");
	}

	std::string patientID = removeSpace(sf.ToString(tPID));

	if (ds.FindDataElement(tSOPUID) && (caseMap.find(patientID) != caseMap.end()))
	{
		int n = caseMap.find(patientID)->second;
		std::string outName(removeSpace(sf.ToString(tSeriesNum)));
		outName.append("_");
		outName.append(sf.ToString(tSOPUID));
		// std::string seriesNum = sf.ToString(tSeriesNum);

		if (strncmp(table[n][7].c_str(), "tick", 4) != 0){
			table[n][7] = "tick";
		}
		std::string bbid = table[n][5];
		std::string diag = table[n][6];
		
		std::map<const gdcm::Tag, std::string> tagMap;
		modifyTags(tagMap, bbid, diag);

		gdcm::Anonymizer modiTags;
		modiTags.SetFile(file);

		for (auto it = ds.Begin(); it != ds.End(); it++)
		{
			// Current Tag Str
			const gdcm::Tag curT_ = it->GetTag();
			std::string curStr_ = sf.ToString(curT_);
			std::ostringstream temp_;
			temp_ << curT_;
			curStr_ = temp_.str();

			if (tagMap.find(curT_) != tagMap.end()){
				modiTags.Replace(curT_, tagMap.find(curT_)->second.c_str());
			}
			else if (curStr_.find("0010") == 1){
				modiTags.Remove(curT_);
			}
		}
		std::string outFileName;
		outFileName.append(outputFolder);

		std::vector<std::string> layers;
		layers.push_back(removeSpace(table[n][5]));
		layers.push_back(replaceSpecialCharacters(removeSpace(studyProtocol)));
		layers.push_back(outName);
		// outFileName.append(removeSpace(table[n][5].c_str()));

		for (int layer = 0; layer < 3; layer++){
			outFileName.append("\\");
			outFileName.append(layers[layer]);
			if (layer == 2)
				break;
			if (CreateDirectory(outFileName.c_str(), NULL)){
				std::cout << "Create Folder: " << outFileName << std::endl;
			}
			else if (ERROR_ALREADY_EXISTS == GetLastError())
			{}
			else{
				std::cout << "Unforeseen " << outFileName << std::endl;
			}
		}
		// std::cout << "output file: " << outFileName.c_str() << std::endl;
		gdcm::Writer writer;
		writer.SetFileName(outFileName.c_str());
		writer.SetFile(file);
		if (! writer.Write()){
			std::cout << "Could not write" << std::endl;
			return std::make_pair(false, "");
		}
		else
			return std::make_pair(true, table[n][1]);
	}
	return std::make_pair(false, "");
}

void parseFolder(std::string inputFolder, std::string outputFolder, std::vector<std::vector<std::string> >& table, std::map<std::string, int> caseMap){
	DIR *pdir = NULL;
	struct dirent *pent = NULL;
	pdir = opendir(inputFolder.c_str());

	if (pdir == NULL)
		return;

	while (pent = readdir(pdir)){
		if (pent == NULL)
			return;
		if ((strcmp(pent->d_name, ".") != 0) && (strcmp(pent->d_name, "..") != 0))
		{
			char pathName[500];
			strncpy_s(pathName, inputFolder.c_str(), sizeof(pathName));
			strncat_s(pathName, "\\", 2);
			// strncat_s(pathName, pent->d_name, strlen(inputFolder.c_str()));
			strncat_s(pathName, pent->d_name, pent->d_namlen);
			if (pent->d_type == isFile){
				std::pair<bool, std::string> info = rwGDCM(pathName, outputFolder.c_str(), table, caseMap);

				if (info.first){
					std::string numStr = table[caseMap.find(info.second)->second][8];
					int num_ = std::stoi(numStr, nullptr, 10);
					table[caseMap.find(info.second)->second][8] = meToString(num_ + 1);
				}
			}
			else{
				parseFolder(pathName, outputFolder, table, caseMap);
			}
		}
	}
}

bool saveCSV(const char *fileName, std::vector<std::vector<std::string> > table)
{
	//std::cout << fileName << std::endl;
	std::ofstream file (fileName, std::ofstream::out);
	for (unsigned int i = 0; i < table.size(); i++)
	{
		file << table[i][0] << "," << table[i][1] << "," << table[i][2] << "," << table[i][3] << "," << table[i][4] << "," << table[i][5] << "," << table[i][6] << "," << table[i][7] << "," << table[i][8] << "," << table[i][9] << std::endl;
	}
	file.close();
	return true;
}
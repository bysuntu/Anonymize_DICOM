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
#include <gdcmSmartPointer.h>

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <windows.h>
#include "dirent.h"
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

class oV{
public:
	bool first;
	int second;
	std::string third;
	oV(bool v0, int v1, std::string v2):first(v0), second(v1), third(v2)
	{
	}
	oV():first(false), second(-1), third("")
	{}

};

std::string removeSpace(std::string Str)
{
	std::string outStr;
	for (unsigned int i=0; i < Str.size(); i++)
	{
		if (!isspace(Str[i]) && (Str[i] != '_'))
		{
			outStr += Str[i];
		}
	}
	return outStr;
}

std::string replaceSpecialCharacters(std::string filepath)
{
	std::string newpath;
	char all[] = { '\\', '*', '"', '<', '>', ':', '|', '?', ' ', '/', '-' };
	std::set<char> characters(all, all + 11);
	for (unsigned int i = 0; i < filepath.size(); i++)
		if (characters.find(filepath[i]) != characters.end())
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
					if ((i == 1 || i == 5) && n > 0)
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

std::set<std::string> removeTags(const char *fileName = "remove"){
	std::set<std::string> tagStr;

	//tagStr.insert("(0010,0030)");
	//tagStr.insert("(0010,0040)");

	tagStr.insert("(0008,0050)");
	tagStr.insert("(0008,0080)");
	tagStr.insert("(0008,0081)");
	tagStr.insert("(0008,1040)");
	tagStr.insert("(0008,1050)");
	tagStr.insert("(0020,0010)");
	tagStr.insert("(0040,0254)");
	
	std::ifstream tagFile;
	tagFile.open(fileName);
	std::string line;
	if (tagFile.is_open()){
		while(getline(tagFile, line)){
			// std::cout << line.c_str() << std::endl;
			tagStr.insert(line);
		}
		tagFile.close();
	}
	return tagStr;
}

std::set<std::string> keepTags(const char *fileName = "protect")
{
	std::set<std::string> tagStr;
	tagStr.insert("(0010,0010)");
	tagStr.insert("(0010,0020)");
	std::ifstream tagFile;
	tagFile.open(fileName);
	std::string line;
	if (tagFile.is_open()){
		while(getline(tagFile, line)){
			// std::cout << line.c_str() << std::endl;
			tagStr.insert(line);
		}
		tagFile.close();
	}
	return tagStr;

}


bool checkFile(const char* fileName, std::string bbid, std::string protocol){
	gdcm::ImageReader reader;
	reader.SetFileName(fileName);
	reader.Read();

	gdcm::File &file = reader.GetFile();
	gdcm::DataSet &ds = file.GetDataSet();
	const gdcm::Global& g = gdcm::Global::GetInstance();
	const gdcm::Dicts &dicts = g.GetDicts();
	const gdcm::Dict &pubdict = dicts.GetPublicDict();

	gdcm::StringFilter sf;
	sf.SetFile(file);

	const gdcm::Tag tStudyProtocol(0x8, 0x1030);
	const gdcm::Tag tPName(0x10, 0x10);
	const gdcm::Tag tPID(0x10, 0x20);
	const gdcm::Tag tDOB(0x10, 0x30);

	std::string studyProtocol = sf.ToString(tStudyProtocol);
	
	if (strncmp(studyProtocol.c_str(), protocol.c_str(), protocol.length()) != 0){
		std::cout << studyProtocol.c_str() << protocol.c_str() << std::endl;
		return false;
	}
	if (strncmp(sf.ToString(tPName).c_str(), bbid.c_str(), bbid.length()) != 0){
		std::cout << "Name" << std::endl;
		std::cout << sf.ToString(tPName).c_str() << bbid.c_str() << std::endl;
		return false;
	}
	if (strncmp(sf.ToString(tPID).c_str(), bbid.c_str(), bbid.length()) != 0){
		std::cout << "PID" << std::endl;
		std::cout << sf.ToString(tPID).c_str() << bbid.c_str() << std::endl;
		return false;
	}
	return true;
}

oV rwGDCM(const char *fileName, const char *outputFolder, std::vector<std::vector<std::string> >& table, std::map<std::string, int> caseMap, 
	const std::set<std::string>& emptyStrings, const std::set<std::string>& keepStrings)
{
	oV result;

	gdcm::ImageReader *reader = new gdcm::ImageReader();
	reader->SetFileName(fileName);
	if (!reader->Read())
	{
		std::cerr << "Could not read: " << fileName << std::endl;
		delete reader;
		return result;
	}
	
	gdcm::File &file = reader->GetFile();
	gdcm::DataSet &ds = file.GetDataSet();
	const gdcm::Global& g = gdcm::Global::GetInstance();
	const gdcm::Dicts &dicts = g.GetDicts();
	const gdcm::Dict &pubdict = dicts.GetPublicDict();
		
	const gdcm::Image &image = reader->GetImage();
	try{
		image.GetPixelFormat().GetScalarTypeAsString();
	}
	catch(...){
		std::cout << fileName << " is not a valid DICOM file" << std::endl;
		delete reader;
		return result;
	}

	gdcm::StringFilter sf;
	sf.SetFile(file);

	const gdcm::Tag tStudyProtocol(0x18, 0x1030);
	const gdcm::Tag tPName(0x10, 0x10);
	const gdcm::Tag tPID(0x10, 0x20);
	const gdcm::Tag tDescrb(0x8, 0x1030);
	const gdcm::Tag tSeriesNum(0x20, 0x11);
	const gdcm::Tag tDOB(0x10, 0x30);
	const gdcm::Tag tWeight(0x10, 0x1030);
	const gdcm::Tag tHeight(0x10, 0x1020);

	gdcm::Tag tSOPUID;
	pubdict.GetDictEntryByName("SOP Instance UID", tSOPUID);

	std::string studyProtocol = sf.ToString(tStudyProtocol);
	if (removeSpace(studyProtocol).size()==0)
	{
		std::cout << "Remove" << std::endl;
		std::cout << fileName << std::endl;
		std::cout << sf.ToString(tPID) << std::endl;
		delete reader;
		return result;
	}

	std::string patientID = removeSpace(sf.ToString(tPID));

	if (ds.FindDataElement(tSOPUID) && (caseMap.find(patientID) != caseMap.end()))
	{
		int n = caseMap.find(patientID)->second;
		std::string outName(replaceSpecialCharacters(sf.ToString(tSeriesNum)));
		outName.append("_");
		outName.append(sf.ToString(tSOPUID));
		// std::string seriesNum = sf.ToString(tSeriesNum);

		if (strncmp(table[n][7].c_str(), "tick", 4) != 0){
			table[n][7] = "tick";
		}
		std::string bbid = table[n][5];
		std::string diag = table[n][6];
		
		gdcm::Anonymizer modiTags;
		modiTags.SetFile(reader->GetFile());
		// std::cout << fileName << std::endl;
		for (auto it = ds.Begin(); it != ds.End(); it++)
		{
			// Current Tag Str
			const gdcm::Tag &curT_ = it->GetTag();
			std::string curStr_ = sf.ToString(curT_);
			std::string curTag_;
			std::ostringstream temp_;
			temp_ << curT_;
			curTag_ = temp_.str();
			// std::cout << "all Tag: " << curTag_.c_str() << curStr_.c_str() << std::endl;
			if (keepStrings.count(curTag_))
			{
				// std::cout << "Tag: " << curTag_.c_str() << curStr_.c_str() << std::endl;
				// modiTags.Replace(curT_, "");
			}
			else if (emptyStrings.count(curTag_))
			{
				modiTags.Replace(curT_, "");
				// std::cout << "e Tag: " << curTag_.c_str() << curStr_.c_str() << std::endl;
				// ds.Remove(curT_);
			}
			else if (curTag_.find("0010") == 1){
				// std::cout << "d Tag: " << curTag_.c_str() << curStr_.c_str() << std::endl;
				ds.Remove(curT_);
			}
		}
		
		modiTags.Replace(tPID, bbid.c_str());
		modiTags.Replace(tPName, bbid.c_str());
		modiTags.Replace(tDescrb, diag.c_str());
		
		std::string outFileName;
		outFileName.append(outputFolder);

		std::vector<std::string> layers;
		layers.push_back(removeSpace(replaceSpecialCharacters(table[n][5])));
		layers.push_back(replaceSpecialCharacters(studyProtocol));
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
		gdcm::Writer* writer = new gdcm::Writer();
		writer->SetFileName(outFileName.c_str());
		writer->SetFile(file);
		if (! writer->Write()){
			std::cout << "Could not write" << std::endl;
			delete writer;
			delete reader;
			return result;
		}
		else{
			result.first = true;
			result.second = n;
			result.third = outFileName;
			delete writer;
			delete reader;
			return result;
		}
	}
	delete reader;
	return result;
}

void parseFolder(std::string inputFolder, std::string outputFolder, std::vector<std::vector<std::string> >& table, std::map<std::string, int> caseMap){
	DIR *pdir = NULL;
	struct dirent *pent = NULL;
	pdir = opendir(inputFolder.c_str());

	if (pdir == NULL)
		return;

	const std::set<std::string> emptyStrings = removeTags();
	const std::set<std::string> keepStrings = keepTags();

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
				// std::cout << pathName << std::endl;
				oV info = rwGDCM(pathName, outputFolder.c_str(), table, caseMap, emptyStrings, keepStrings);

				if (info.first && checkFile(info.third.c_str(), table[info.second][5], table[info.second][6])){
					std::string numStr = table[info.second][8];
					int num_ = std::stoi(numStr, nullptr, 10);
					table[info.second][8] = meToString(num_ + 1);
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
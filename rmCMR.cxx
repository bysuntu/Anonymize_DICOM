#define    __DEV_CPP__ // to resolve FLTK dirent.h conflict
//FL
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Widget.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/filename.H>
#include <FL/Fl_JPEG_Image.H>
#include <FL/Fl_Image.H>
#include <FL/FL_DOUBLE_Window.H>
#include <FL/Fl_Shared_Image.H>
#include <FL/Fl_Counter.H>
#include <FL/Fl_Value_Slider.H>
#include <FL/Fl_Hor_Value_Slider.H>
#include <FL/Fl_Valuator.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Group.H>
//OpenGL
#include <FL/gl.h>
#include <FL/Fl_GL_Window.H>

//Additional

#include <string.h>
//#include <sstream>
//#include <array>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <sys/stat.h>
#include <ctype.h>
#include <windows.h>
//#include "dirent.h"

#include <vector>
#include <map>

#include "anonymize.h"
#include "gui.h"

int main(int argc, char* argv[]){
	if (argc == 7){
		std::string csvFileName, inputDirName, outputDirName;
		for (int i = 0; i < argc; i++){
			// std::cout << argv[i] << std::endl;
			if (strcmp(argv[i], "-i") == 0){
				std::cout << "csv: " << argv[i + 1] << std::endl;
				csvFileName = argv[i + 1];
			}
			if (strcmp(argv[i], "-s") == 0){
				std::cout << "source: " << argv[i + 1] << std::endl;
				inputDirName = argv[i + 1];
			}
			if (strcmp(argv[i], "-t") == 0){
				std::cout << "target: " << argv[i + 1] << std::endl;
				outputDirName = argv[i + 1];
			}
		}
		std::vector<std::vector<std::string> > table;
		std::map<std::string, int> caseMap = loadCSV(csvFileName.c_str(), table);
		parseFolder(inputDirName, outputDirName, table, caseMap);
		saveCSV(csvFileName.c_str(), table);
	}
	else
		gui();
	return 0;
}
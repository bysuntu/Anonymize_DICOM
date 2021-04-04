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

#include <string>
#include <map>
#include <vector>

#include "anonymize.h"

void pickInputDir_cb(Fl_Widget* o, void* p)
{
	Fl_Input* inputDCMFolder = (Fl_Input*)p;
	Fl_Native_File_Chooser native;
	
	native.title("Pick a Directory");
	native.directory(inputDCMFolder->value());
	native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);

	// Show native chooser
	switch (native.show()) {
	case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
	case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
	default: 								// PICKED DIR
		if (native.filename()) {
			inputDCMFolder->value(native.filename());
		}
		else {
			inputDCMFolder->value("NULL");
		}
		break;
	}
	o->parent()->redraw();
	//std::cout << fltkDCMFolder->value() << std::endl;
}

void pickOutputDir_cb(Fl_Widget* o, void* p)
{
	Fl_Input* outputDCMFolder = (Fl_Input*)p;
	Fl_Native_File_Chooser native;
	native.title("Pick a Directory");
	native.directory(outputDCMFolder->value());
	native.type(Fl_Native_File_Chooser::BROWSE_DIRECTORY);

	// Show native chooser
	switch (native.show()) {
	case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
	case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
	default: 								// PICKED DIR
		if (native.filename()) {
			outputDCMFolder->value(native.filename());
		}
		else {
			outputDCMFolder->value("NULL");
		}
		break;
	}
	o->parent()->redraw();
	//std::cout << fltkDCMFolder->value() << std::endl;
}

void pickCSVFile_cb(Fl_Widget* o, void* p)
{
	std::cout << "HEre" << std::endl;
	Fl_Input* fltkCSVFile = (Fl_Input*)p;
	Fl_Native_File_Chooser native;
	native.title("Pick a File");
	//native.directory(saFolderName->value());

	native.type(Fl_Native_File_Chooser::BROWSE_FILE);
	native.filter("CSV\t*.csv\n");

	// Show native chooser
	switch (native.show()) {
	case -1: fprintf(stderr, "ERROR: %s\n", native.errmsg()); break;	// ERROR
	case  1: fprintf(stderr, "*** CANCEL\n"); fl_beep(); break;		// CANCEL
	default: 								// PICKED DIR
		if (native.filename()) {
			//saFolderName->value(native.filename());
			fltkCSVFile->value(native.filename());
		}
		else {
			//saFolderName->value("NULL");
			fltkCSVFile->value("NULL");
		}
		break;
	}
	o->parent()->redraw();
	//std::cout << fltkCSVFile->value() << std::endl;
}

void close_cb(Fl_Widget* o, void*)
{
	std::exit(0);
}


void gui(){
	std::string csvFileName, inputDirName, outputDirName;
	std::vector<std::vector<std::string> > table;

	Fl_Double_Window window(900, 200, 600, 300);
	window.color(FL_WHITE);
	window.border(1);
	window.begin();

	// Fl_Input *inputDCMFolder, *outputDCMFolder, *fltkCSVFile;

	Fl_Input* inputDCMFolder = new Fl_Input(100, 50, 400, 30, "MRI_Input");
	Fl_Button *dcmInDir = new Fl_Button(500, 50, 70, 30, "Directory");

	Fl_Input* outputDCMFolder = new Fl_Input(100, 100, 400, 30, "MRI_Output");
	Fl_Button *dcmOutDir = new Fl_Button(500, 100, 70, 30, "Directory");

	Fl_Input* fltkCSVFile = new Fl_Input(100, 150, 400, 30, "CSV");
	Fl_Button *csvFile = new Fl_Button(500, 150, 70, 30, "File");

	Fl_Button *start = new Fl_Button(200, 200, 50, 50, "Start");
	start->down_color(FL_RED);
	start->color(FL_GREEN);
	Fl_Button *quit = new Fl_Button(350, 200, 50, 50, "Quit");

	window.end();
	window.show();

	while (Fl::wait())
	{
		dcmInDir->callback(pickInputDir_cb, inputDCMFolder);
		dcmOutDir->callback(pickOutputDir_cb, outputDCMFolder);
		csvFile->callback(pickCSVFile_cb, fltkCSVFile);
		quit->callback(close_cb);
		if (start->value())
		{
			const char *csvFileName = fltkCSVFile->value();
			std::map<std::string, int> caseMap = loadCSV(csvFileName, table);
			inputDirName.clear();
			inputDirName.append(inputDCMFolder->value());
			outputDirName.clear();
			outputDirName.append(outputDCMFolder->value());

			parseFolder(inputDirName, outputDirName, table, caseMap);

			std::cout << "!!!!!!!!!!!FINISHED!!!!!!!!!!!!!!!!!!" << std::endl;
		}
	}

}
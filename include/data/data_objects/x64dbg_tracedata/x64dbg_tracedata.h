#pragma once
#include "trace64data_buf.h"
#include <Windows.h>
namespace ns_xtr {

	//		INPUT FILE DATA (PROPERTIES)
	/*
	* 	Responsible for dealing with all other aspects regarding 
	*	the input file:
	*					1) file name
	*					2) file size
	*					3) misc. info that help 'define' the file
	* 
	*	which, in this case, is the json blob at the beginning of 
	*	every .trace64 file.
	*/
	class x64dbg_tracedata : public trace64data_buf {
	public:

		x64dbg_tracedata(const char* file_name); // wrapped w/ try-catch 
		~x64dbg_tracedata();

		std::string& getFileName()  { return _file_name; };
		std::string& getJSONBlob()  { return _jsonblob;  };
		_QWORD getFileSize()	    { return _file_size; };
		FEXTBT getFileExt()			{ return _fileExt;   };

		void _saveJsonData(const std::string& jsonblob);
		bool _atEndOfFile();

		void _cleanup_x64dbg_tracedata();

		
	private:
		// basic file properties
		std::string _file_name;
		std::string _jsonblob;
		_QWORD _file_size;
		FEXTBT _fileExt;
		HANDLE _tracefile_handle = nullptr;
		bool _validFileHandleAndSize = true;


		bool _saveFileSize();
		bool _saveFileHandle();
		void _tryToSaveFileHandleAndSize();

		bool _readFile();
		void _tryToReadInputFile();
		void _checkFileExt();

	};

}
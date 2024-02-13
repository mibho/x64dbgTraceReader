#include "../../../../include/data/data_objects/x64dbg_tracedata/x64dbg_tracedata.h"

ns_xtr::x64dbg_tracedata::x64dbg_tracedata(const char* file_name) try : _file_size(0),
																	    _file_name(file_name),
																		_fileExt(EXT_TYPE_UNK)
{
	_tryToSaveFileHandleAndSize();

	if (_validFileHandleAndSize) // default true. false iff above fn fails
	{

		std::cout << "[+] - File handle obtained! " << std::endl;

		_file_contents = new BYTE[_file_size];
		_zero_out_buf(_file_size);

		_checkFileExt();
		_tryToReadInputFile();

		std::cout << "\tx64dbg_tracedata object successfully created!" << std::endl;
	}
	else 
	{
		throw std::runtime_error(constants::errors::FAIL_CREATE_X64DBG_TRACEDATA);
	}

}
catch (const std::exception& err) 
{ 
	std::cerr << err.what() << std::endl; 
	std::cout << "Failed in x64dbg_tracedata constructor!" << std::endl; 
};

void ns_xtr::x64dbg_tracedata::_cleanup_x64dbg_tracedata()
{
	_cleanupBufIfNeeded();

	if (_tracefile_handle != nullptr)
	{
		if (_tracefile_handle != INVALID_HANDLE_VALUE)
			CloseHandle(_tracefile_handle);

		_tracefile_handle = nullptr;
	}

}


ns_xtr::x64dbg_tracedata::~x64dbg_tracedata()
{
	_cleanup_x64dbg_tracedata();
}

bool ns_xtr::x64dbg_tracedata::_atEndOfFile()
{
	return _current_pos >= _total_bytes_read;
}

void ns_xtr::x64dbg_tracedata::_saveJsonData(const std::string& jsonblob)
{
	_jsonblob = jsonblob;
}

bool ns_xtr::x64dbg_tracedata::_saveFileSize()
{
	bool success = GetFileSizeEx(_tracefile_handle, reinterpret_cast<PLARGE_INTEGER>(&_file_size));

	if (!success)
	{
		_DWORD error = GetLastError();

		std::cout << "[!] - Error code " << error << ": " << "Failed to get file size!" << std::endl;
	}

	return success;
}

bool ns_xtr::x64dbg_tracedata::_saveFileHandle()
{
	bool success = true;

	_tracefile_handle = CreateFileA(_file_name.c_str(), 
									GENERIC_READ, 
									FILE_SHARE_READ, 
									NULL, 
									OPEN_EXISTING, 
									FILE_ATTRIBUTE_NORMAL, NULL);

	if (_tracefile_handle == INVALID_HANDLE_VALUE) {

		_DWORD error = GetLastError();

		// TODO way later: provide info corresponding to error code?
		if (error == ERROR_FILE_NOT_FOUND)
			std::cout << "[!] - Error code " << error << ": file not found!" << std::endl;
		else
			std::cout << "[!] - Error code " << error << std::endl;

		success = false;
	}

	return success;
}

void ns_xtr::x64dbg_tracedata::_tryToSaveFileHandleAndSize()
{
	try 
	{
		if (!_saveFileHandle() || !_saveFileSize()) 
		{
			throw std::runtime_error(constants::errors::FAIL_GET_VALID_INPUT_FILE_HANDLE);
		}
		else
		{
			if (_file_size <= 0)
			{
				throw std::runtime_error(constants::errors::EMPTY_OR_INVALID_INPUT_FILE_SIZE);
			}
		}


	}
	catch (const std::exception &err) 
	{
		std::cerr << "[!] - Error in x64dbg_tracedata constructor! " << std::endl;
		std::cerr << err.what() << std::endl;
		_validFileHandleAndSize = false;

	}
}

bool ns_xtr::x64dbg_tracedata::_readFile()
{
	bool success = false;

	if (_file_contents != nullptr) 
	{

		success = ReadFile(_tracefile_handle, 
						   _file_contents, 
						   _file_size, 
						   reinterpret_cast<LPDWORD>(&_total_bytes_read), NULL);
	}

	return success;
}

void ns_xtr::x64dbg_tracedata::_tryToReadInputFile()
{
	try
	{
		if (_readFile())
		{
			std::cout << "\tSuccessfully read " << _total_bytes_read << " bytes." << std::endl;
		}
		else 
		{
			std::cout << "\tFailed to read data of the provided file! Exiting..." << std::endl;
			throw std::runtime_error(constants::errors::FAIL_GET_VALID_INPUT_FILE_HANDLE);
		}
	}
	catch (const std::exception &err)
	{
		std::cout << "[!] - Error in x64dbg_tracedata constructor! " << std::endl;
		std::cerr << err.what() << std::endl;
	}

}

void ns_xtr::x64dbg_tracedata::_checkFileExt()
{
	try
	{
		_DWORD file_name_len = _file_name.length();

		if (file_name_len > constants::TRACE64_MINIMUM_FILE_EXT_LENGTH)
		{
			_DWORD dotIndex = file_name_len - constants::TRACE64_MINIMUM_FILE_EXT_LENGTH;
			_DWORD lastCharIndex = file_name_len - 1;
			_DWORD secondLastCharIndex = file_name_len - 2;

			if (_file_name[dotIndex] == '.'
				&&
			   (_file_name[secondLastCharIndex] == '3' && _file_name[lastCharIndex] == '2')
				||
			   (_file_name[secondLastCharIndex] == '6' && _file_name[lastCharIndex] == '4'))
			{

				_fileExt = (_file_name[secondLastCharIndex] == '3') ? EXT_TYPE_32 : EXT_TYPE_64;
			}
			else
			{
				std::cout << "\tFailed to detect if 32-bit or 64-bit x64dbg tracefile! Exiting..." << std::endl;
				throw std::runtime_error(constants::errors::FAIL_GET_VALID_INPUT_FILE_HANDLE);
			}

			switch (_fileExt)
			{
			case EXT_TYPE_32:
				std::cout << ".trace32 format" << std::endl;
				break;
			case EXT_TYPE_64:
				std::cout << ".trace64 format" << std::endl;
				break;
			default:
				std::cout << "Unknown format.??" << std::endl;
				break;

			}
			
		}
		else
		{
			std::cout << "\tFailed to read data of the provided file! Exiting..." << std::endl;
			throw std::runtime_error(constants::errors::FAIL_GET_VALID_INPUT_FILE_HANDLE);
		}
	}
	catch (const std::exception& err)
	{
		std::cout << "[!] - Error in x64dbg_tracedata constructor! " << std::endl;
		std::cerr << err.what() << std::endl;
	}
}


#include "../../../../include/data/data_objects/x64dbg_tracedata/trace64data_buf.h"	

#include <algorithm> // std::fill() 

using namespace ns_xtr::dtypes;

bool ns_xtr::trace64data_buf::_can_read(_BYTE read_amt)
{
	return ((((_current_pos + read_amt)) <= _total_bytes_read)  // just checks if end of buf reached
						&& (_current_pos < _total_bytes_read));
}

_BYTE ns_xtr::trace64data_buf::_read_next_byte()
{
	if (_can_read(constants::LEN_BYTE))
	{
		_allgood = true;
		return *reinterpret_cast<_BYTE*>((_file_contents + _current_pos));
	}

	// if ret val 0 and allgood == false; invalid.
	_allgood = false;
	return 0;
}

_WORD ns_xtr::trace64data_buf::_read_next_word()
{
	if (_can_read(constants::LEN_WORD))
	{
		_allgood = true;
		return *reinterpret_cast<_WORD*>((_file_contents + _current_pos));
	}

	// if ret val 0 and allgood == false; invalid.
	_allgood = false;
	return 0;
}

_DWORD ns_xtr::trace64data_buf::_read_next_dword()
{
	if (_can_read(constants::LEN_DWORD))
	{
		_allgood = true;
		return *reinterpret_cast<_DWORD*>((_file_contents + _current_pos));
	}

	// if ret val 0 and allgood == false; invalid.
	_allgood = false;
	return 0;
}

_QWORD ns_xtr::trace64data_buf::_read_next_qword()
{
	if (_can_read(constants::LEN_QWORD))
	{
		_allgood = true;
		return *reinterpret_cast<_QWORD*>((_file_contents + _current_pos));
	}

	// if ret val 0 and allgood == false; invalid.
	_allgood = false;
	return 0;
}

_QWORD ns_xtr::trace64data_buf::_readDataSize(_BYTE read_amt)
{
	_QWORD result = 0LL;

	switch (read_amt) 
	{
	case constants::LEN_BYTE:
		result = _read_next_byte();
		break;
	case constants::LEN_WORD:
		result = _read_next_word();
		break;
	case constants::LEN_DWORD:
		result = _read_next_dword();
		break;
	case constants::LEN_QWORD:
		result = _read_next_qword();
		break;
	default:
		_allgood = false;
		break;
	}

	if (_allgood) {
		_current_pos += read_amt;
	}
	else {
		std::cout << "failed to read @ " << _current_pos << "/" << _total_bytes_read << std::endl;
	}

	//std::cout << "curr pos: " << _current_pos << " " << static_cast<int>(result) << std::endl;
	return result;

}

_BYTE ns_xtr::trace64data_buf::_readByte()
{
	_BYTE content = _read_next_byte();

	if (_allgood) {
		_current_pos += constants::LEN_BYTE;
	}
	else {	// dont think this a possible edge case. check later
		std::cout << "ERROR! EXITING..." << std::endl;
		std::cout << "current file position: " << _current_pos << std::endl;
		exit(EXIT_FAILURE);
	}

	return content;
}



void ns_xtr::trace64data_buf::_zero_out_buf(_QWORD buffer_size)
{
	std::fill((_file_contents),
			(_file_contents + (buffer_size - 1)),
			constants::ZERO_BYTE);

	_current_pos = 0;
	
}

void ns_xtr::trace64data_buf::_cleanupBufIfNeeded()
{
	if (_file_contents != nullptr) 
	{
		delete _file_contents;
		_file_contents = nullptr;
	}
}

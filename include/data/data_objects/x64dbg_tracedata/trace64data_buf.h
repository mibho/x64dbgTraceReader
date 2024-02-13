#pragma once
#include "../../data_formats.h"

namespace ns_xtr {

/*	
	*	 Input file buffer stuff
	*	-------------------------
	*		- Input file bytes read
	*		- responsible for adjusting file pointer to 'process' specific # of bytes
	*		  until the entire file is read.
	* 
	*	(alternatively use fstream and std lib to read file.)
	* 
	*	BASICALLY ADVANCES FILE POINTER UNTIL END OF FILE
*/
	class trace64data_buf {
	public:

		trace64data_buf()
						 {};
		~trace64data_buf() 
						 { _cleanupBufIfNeeded(); };

		_QWORD _readDataSize(_BYTE read_amt); // read 1,2,4, or 8 bytes
		_BYTE  _readByte(); // read 1 byte
		void _zero_out_buf(_QWORD buffer_size);	
		void _cleanupBufIfNeeded();

		_BYTE* _file_contents = nullptr;
		_QWORD _current_pos = 0;
		_QWORD _total_bytes_read = 0;
		_DWORD _current_ins_block_processed = 0;
		

	private:

		bool _allgood = false;	// did read succeed?
		bool _can_read(_BYTE read_amt);

		_BYTE  _read_next_byte();
		_WORD  _read_next_word();
		_DWORD _read_next_dword();
		_QWORD _read_next_qword();
	};


}
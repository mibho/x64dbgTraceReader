#pragma once

#include "../data/data_objects/xtr_tracedata/xtr_tracedata.h"
#include "../data/data_objects/x64dbg_tracedata/x64dbg_tracedata.h"


namespace ns_xtr {

	/*
	* MUST check memtype value if retrieving memory access data
	* MEMATYPE_WRITE = all valid
	* MEMATYPE_READ = new_val not valid
	*/
	struct memadata
	{
		MEMATYPE memtype = MEMATYPE_NOTSET;
		_QWORD address = 0;
		_QWORD old_val = 0;
		_QWORD new_val = 0;

	};

	using namespace dtypes;

	class data_manager {
	protected:
		data_manager(const char* file_name);
		~data_manager() { _cleanup_data_manager(); };

		// orig file bytes/data. dont worry if not changing 'internal' stuff.
		x64dbg_tracedata* _provided_file = nullptr; 

		// main data
		xtr_tracedata* xtrTrace = nullptr;

		// responsible for order of new .trace file data 
		std::vector<_DWORD> entries_to_keep;



		bool xtrToTrace64File(const char* filename); // see _TRACE64FORMAT_adjustEntryContext()
		bool createNewTraceFile(const char* filename);

		// CALLER IS RESPONSIBLE FOR ENSURING INDEX IS VALID. otherwise ret vector of 0s.
		std::vector<_QWORD> getAllRegisterDataForEntry(_DWORD Nth_entry);
		bool doesEntryHaveMemaData(_DWORD Nth_entry);
		std::vector<ns_xtr::memadata> getEntryMemaData(_DWORD Nth_entry);	
		
		// this is the only _ prefixed function to worry about. NOT required but re-calculates RLE-compressed data
		void _TRACE64FORMAT_adjustEntryContext();
		bool processFile();
		
		// init related; processFile() deals w/ setup so dont worry unless changing 'internal' stuff.
		bool _isSetUp = false;
		bool _failed = false;
		bool _setupXTRTraceData();
		void _trace64ToXTR();
		void _cleanup_data_manager();
		
		// use to implement own format
		bool _createNewOutputFile(const char* filename);

		


		// input: list of vals for what regs changed. output: corresponding reg, new val
		// by default, x64dbg_trace_block regchanges uses absolute + relative indexing
		std::vector<_BYTE> getAbsoRelativeForm(const std::vector<REGISTERCONTEXT>& changelist) 
		{
			std::vector<_BYTE> absorelative;
			
			_BYTE absorelativeindex = 0;

			for (_DWORD i = 0; i < changelist.size(); i++) 
			{
				// i == 0? ret 1st element. else: ret (curr element - prev element - 1)
				absorelativeindex = (i == 0) ? static_cast<_BYTE>(changelist[0]) : (static_cast<_BYTE>(changelist[i]) - static_cast<_BYTE>(changelist[i - 1]) - 1);
				absorelative.push_back(absorelativeindex);
			}

			return absorelative;
		}
		// convert x64dbg 'absorelative' format. 
		std::vector<REGISTERCONTEXT> getRCForm(const std::vector<_BYTE>& changelist) 
		{
			_BYTE absorelativeindex = 0;
			_BYTE prev_val = 0;
		
			std::vector<REGISTERCONTEXT> affected_list;

			for (_DWORD i = 0; i < changelist.size(); i++)
			{
				absorelativeindex = (i == 0) ? changelist[0] : prev_val + changelist[i] + 1;
				prev_val = absorelativeindex;
				if (absorelativeindex <= RC171)
				{
					switch (absorelativeindex)
					{
					case 0:  affected_list.push_back(RAX); break;
					case 1:	 affected_list.push_back(RCX); break;
					case 2:	 affected_list.push_back(RDX); break;
					case 3:	 affected_list.push_back(RBX); break;
					case 4:	 affected_list.push_back(RSP); break;
					case 5:	 affected_list.push_back(RBP); break;
					case 6:  affected_list.push_back(RSI); break;
					case 7:	 affected_list.push_back(RDI); break;
					case 8:	 affected_list.push_back(R8);  break;
					case 9:	 affected_list.push_back(R9);  break;
					case 10: affected_list.push_back(R10); break;
					case 11: affected_list.push_back(R11); break;
					case 12: affected_list.push_back(R12); break;
					case 13: affected_list.push_back(R13); break;
					case 14: affected_list.push_back(R14); break;
					case 15: affected_list.push_back(R15); break;
					case 16: affected_list.push_back(RIP); break;
					case 17: affected_list.push_back(RFLAGS); break;
					case 18: affected_list.push_back(GS);  break;
					case 19: affected_list.push_back(FS);  break;
					case 20: affected_list.push_back(ES);  break;
					case 21: affected_list.push_back(DS);  break;
					case 22: affected_list.push_back(CS);  break;
					case 23: affected_list.push_back(SS);  break;
					case 24: affected_list.push_back(DR0); break;
					case 25: affected_list.push_back(DR1); break;
					case 26: affected_list.push_back(DR2); break;
					case 27: affected_list.push_back(DR3); break;
					case 28: affected_list.push_back(DR6); break;
					case 29: affected_list.push_back(DR7); break;

					default:	// find remaining RC vals that correspond to each index; TEMP 'FIX'
						affected_list.push_back(static_cast<REGISTERCONTEXT>(absorelativeindex));
						continue;
					}
				}
			}

			return affected_list;
		};

		
	private:

		// used in converting x64dbg block to xtr_data
		bool _preserveData = false;
		_DWORD _prev_TID = 0;
		_DWORD _new_entry_count = 0;
		_QWORD _entry_cip = 0;
		

		bool _filterCalled = false;
		bool _fullContextInitialized = false;
		x64dbg_trace_block _full_entry_context;
		// create a copy of logged vals.
		void _copyInitialFullContext()
		{
			if (xtrTrace != nullptr)
			{
				if (xtrTrace->doesNthEntryExist(0))
				{
					auto& first_entry = xtrTrace->getInstructionFromNthEntry(0);
					_full_entry_context = first_entry._get_trace_block();
				}
			}
		}
		void _TRACE64FORMAT_updateAllExceptRC(const x64dbg_trace_block& entry);

		// new file stuff
		HANDLE _newfile = nullptr;
		std::string _newjsonblob;
		bool _copyBlobData();
		std::vector<BYTE> _newfile_databuf;
		void _appendToNewFileBuf(_BYTE* srcval, _QWORD datalen);
		void _appendToNewFileBuf(_BYTE srcval);
		void _appendToNewFileBuf(_WORD srcval);
		void _appendToNewFileBuf(_DWORD srcval);
		void _appendToNewFileBuf(_QWORD srcval);
		void _clearNewFileBuf();

		// deal w/ allocated resources
		void _free_and_close_provided_file_if_needed();
		void _free_xtrTrace_if_needed();
		void _free_and_close_new_file_if_needed();
		


		// confirm .trace file format
		bool _isMagicBytesSigFound();
		bool _processJSONBlob();
		bool _confirmTrace64File();


		// ensure data properly rearranged so can reconstruct correct vals after filter
		void _TRACE64FORMAT_getNewTraceOrder();
		void _TRACE64FORMAT_reLinkNewEntries();
		void _TRACE64FORMAT_redirectValidContext(_DWORD last_valid_index, _DWORD next_valid_index);
		
/*
		* fns to convert .trace64 data into data usable by x64dbgTraceReader
		*
		*				TRACE64 -> XTR FORMAT
		*				---------------------
*/
		xtr_data _TRACE64FORMAT_convertToXTREntry(const x64dbg_trace_block& entry);
		void _TRACE64FORMAT_BYTE1_readBlockType(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_BYTE2_readRegisterChanges(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_BYTE3_readMemoryAccesses(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_BYTE4_readBlockFlagsAndOpcodeSize(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_BYTE5_readRegisterChangePosition(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_QWORD6_readRegisterChangeNewData(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_BYTE7_readMemoryAccessFlags(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_QWORD8_readMemoryAccessAddresses(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_QWORD9_readMemoryAccessOldData(x64dbg_trace_block& entry);
		void _TRACE64FORMAT_QWORD10_readMemoryAccessNewData(x64dbg_trace_block& entry);

		/*
		* fns to convert x64dbgTraceReader entries into .trace64 data
		* 
		*				XTR -> TRACE64 FORMAT
		*				---------------------
		* 
		*/
		_DWORD _TRACE64FORMAT_writeAsTrace64Entry(_DWORD Nth_Entry);

		void _TRACE64FORMAT_HEADER1_writeMagicBytes();
		void _TRACE64FORMAT_HEADER2_writeJsonData();

		void _TRACE64FORMAT_BYTE1_writeBlockType(_DWORD Nth_Entry);
		void _TRACE64FORMAT_BYTE2_writeRegisterChanges(_DWORD Nth_Entry);
		void _TRACE64FORMAT_BYTE3_writeMemoryAccesses(_DWORD Nth_Entry);
		void _TRACE64FORMAT_BYTE4_writeBlockFlagsAndOpcodeSize(_DWORD Nth_Entry);
		void _TRACE64FORMAT_BYTE5_writeRegisterChangePosition(_DWORD Nth_Entry);
		void _TRACE64FORMAT_QWORD6_writeRegisterChangeNewData(_DWORD Nth_Entry);
		void _TRACE64FORMAT_BYTE7_writeMemoryAccessFields(_DWORD Nth_Entry);

	};

}
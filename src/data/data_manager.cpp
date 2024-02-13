#include "../../include/data/data_manager.h"

using namespace ns_xtr::dtypes;

ns_xtr::data_manager::data_manager(const char* file_name) {

	try
	{
		// init xtr_tracedata later. first need to format trace64 data
		if (_provided_file == nullptr) 
		{
			std::cout << "data man constructor called. _provided_file = nullptr." << std::endl;
			_provided_file = new x64dbg_tracedata(file_name);

		}

		std::cout << "data man constructor success!." << std::endl;
	}

	catch (...)
	{
		_failed = true;
		// output msg then let implicit rethrow 
		std::cerr << constants::errors::FAIL_CREATE_DATA_MANAGER << std::endl;

	}
}

void ns_xtr::data_manager::_free_and_close_provided_file_if_needed() 
{
	if (_provided_file != nullptr) 
	{
		_provided_file->_cleanup_x64dbg_tracedata();
		delete _provided_file;
		_provided_file = nullptr;
	}
}

void ns_xtr::data_manager::_free_xtrTrace_if_needed()
{
	if (xtrTrace != nullptr) 
	{
		delete xtrTrace;
		xtrTrace = nullptr;
	}
}

void ns_xtr::data_manager::_free_and_close_new_file_if_needed()
{
	if (_newfile != nullptr && _newfile != INVALID_HANDLE_VALUE) 
	{
		CloseHandle(_newfile);
		_newfile = nullptr;
	}
}

void ns_xtr::data_manager::_cleanup_data_manager()
{
	_free_and_close_provided_file_if_needed();
	_free_xtrTrace_if_needed();
	_free_and_close_new_file_if_needed();
}


bool ns_xtr::data_manager::_setupXTRTraceData()
{
	if (!_isSetUp) {

		if (xtrTrace == nullptr) 
		{
			xtrTrace = new xtr_tracedata();
		}

		_isSetUp = true;
	}

	return _isSetUp && (xtrTrace != nullptr);
}

bool ns_xtr::data_manager::_copyBlobData()
{
	if (_provided_file != nullptr)
	{
		_newjsonblob = _provided_file->getJSONBlob();
		return true;
	}

	return false;
}


// Part 1 (trace format): is first 4 bytes == TRAC
bool ns_xtr::data_manager::_isMagicBytesSigFound()
{
	bool success = false;
	_DWORD magic_bytes = 0;

	magic_bytes = _provided_file->_readDataSize(constants::TRACE64_FILE_MAGIC_SIG_LENGTH);

	// ensure 'magic' sig TRAC is present: checking 1st 4 magic bytes (is == TRAC?) 
	success = (magic_bytes == constants::TRACE64_FILE_MAGIC_BYTES_LITTLE_ENDIAN);

	return success;
}

// Part 2 (trace format): ensure next field is JSON data
bool ns_xtr::data_manager::_processJSONBlob()
{
	bool success = false;
	std::string blob;
	_DWORD blob_length = _provided_file->_readDataSize(constants::TRACE64_FILE_JSON_BLOB_HEADER_SIZE);

	if (!_provided_file->_atEndOfFile()) {

		for (_DWORD i = 0; i < blob_length; i++)
		{
			blob += _provided_file->_readByte();
		}

		_provided_file->_saveJsonData(blob);
	}

	// need fix; check for valid json data with nlohmann lib but for now, jst check file is at least greater than len of json data 
	success = (blob.length() == blob_length);

	return success;
}

// check that input file header adheres to trace64 file format
bool ns_xtr::data_manager::_confirmTrace64File()
{
	return (_isMagicBytesSigFound() && _processJSONBlob());
}


// MUST CALL BEFORE ATTEMPTING TO PROCESS ENTRIES! 
bool ns_xtr::data_manager::processFile()
{
	if (!_setupXTRTraceData()) // init xtr_tracedata
	{
		std::cout << "failed to init XTRTraceData obj" << std::endl;
		return false;
	}

	if (_confirmTrace64File()) // magic 'TRAC' and json blob detected?
	{
		if (!_copyBlobData())
			return false;

		while (!_provided_file->_atEndOfFile()) // convert entries until end
		{
			_trace64ToXTR();
		}

		_copyInitialFullContext(); // 1st entry is MO512; requires all data. also avoids bigger headache
	}

	return true;

}


bool ns_xtr::data_manager::_createNewOutputFile(const char* filename)
{
	bool invalidFileName = (filename == nullptr || strlen(filename) == 0);

	if (invalidFileName)
	{
		std::string default_name = constants::DEFAULT_OUTPUT_FILE_NAME;

		_newfile = CreateFileA(default_name.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE,
																	NULL, CREATE_ALWAYS,
																	FILE_ATTRIBUTE_NORMAL, NULL);
		std::cout << "output name not provided. file will be created with default name: " << default_name << std::endl;
	}
	else
	{
		_newfile = CreateFileA(filename, GENERIC_WRITE, FILE_SHARE_WRITE,
														NULL, CREATE_ALWAYS,
														FILE_ATTRIBUTE_NORMAL, NULL);
	}

	bool success = (_newfile != INVALID_HANDLE_VALUE);

	return success;
}

bool ns_xtr::data_manager::createNewTraceFile(const char* filename)
{
	bool fileCreated = false;

	std::string default_name = filename;

	if (_provided_file->getFileExt() == EXT_TYPE_32)
		default_name += ".trace32";
	else
		default_name += ".trace64";

	fileCreated = _createNewOutputFile(filename);

	return fileCreated;
}


bool ns_xtr::data_manager::xtrToTrace64File(const char* filename)
{
	bool success = false;

	// if it's not initialized, then trace64 data wasnt processed properly
	if (xtrTrace != nullptr) 
	{
		if (_prev_TID) // reset val if was used when processing input data 
			_prev_TID = 0;

		bool newFileGood = createNewTraceFile(filename);
		
		// create new output file, get its handle and original json data.
		if (newFileGood) 
		{
			_TRACE64FORMAT_HEADER1_writeMagicBytes();
			_TRACE64FORMAT_HEADER2_writeJsonData();

			_QWORD total_bytes_written = 0;
			_QWORD number_of_entries = entries_to_keep.size();	// filtered list

			
			for (_DWORD i = 0; i < number_of_entries; i++)
			{	
				total_bytes_written += _TRACE64FORMAT_writeAsTrace64Entry(entries_to_keep[i]);
			}
		
			
			success = (total_bytes_written != 0);

			if (success)
			{
				std::cout << "complete! wrote " 
						  << total_bytes_written << " bytes to " << filename <<  std::endl;
			}
			else
			{
				std::cout << "failed! zero bytes written to file..." << std::endl;
			}
			
			std::cout << "closing handle of newfile..." << std::endl;
			CloseHandle(_newfile);
		}
		else
		{
			std::cout << "failed to create a new file with the name: " << filename << std::endl;
		}
	}

	return success;
}


/*
* solely for dealing with _full_entry_context (all register vals)
* 
* maintains appropriate values for the _full_entry_context object, which is responsible for 
* representing the state of registers at that particular instruction BEFORE including the
* effects resulting from the current instruction. only relevant when entry is a multiple of 512.
*/
void ns_xtr::data_manager::_TRACE64FORMAT_updateAllExceptRC(const x64dbg_trace_block& entry)
{
	if (_fullContextInitialized)
	{
		// maintain rc count then propagate changes to rcdata
		_full_entry_context.ThreadID = entry.ThreadID;
		_full_entry_context.opcodes = entry.opcodes;
		_full_entry_context.mem_acc_addr = entry.mem_acc_addr;
		_full_entry_context.mem_acc_flags = entry.mem_acc_flags;
		_full_entry_context.mem_acc_new = entry.mem_acc_new;
		_full_entry_context.mem_acc_old = entry.mem_acc_old;
		_full_entry_context.MemoryAccesses = entry.MemoryAccesses;
		_full_entry_context.mem_access_entries = entry.mem_access_entries;
	}
	else // init
	{
		_full_entry_context = entry;
		_fullContextInitialized = true;
	}
}
/*
 * Creates a xtr_data object from a x64dbg_trace_block (formatted data).
 * 
 * 
 * 
 *	NOTE: specific REGISTERCONTEXT values are documented poorly?.. 
 *	indices 0-29 are clear but something like index 136..? most likely FLAGS related since there's a diff in regdata only when FLAGS changes.
 * 
 * 
 *		core idea:
 *			1) deal with absorelative indices and account for MO512 full context requirement
 *				- done by converting entries w/ full context to just their respective changes. 'removes' constraint of MO512 needing full context 
 *			2) keep full context copy starting from very beginning and apply changes as needed for each entry
 *				- if current entry happens to have full context, just copy that over and repeat til end
 *			3) separately set aside MO512 entries (in _originalMO512Entries in xtr_tracedata.h) <- done to simplify retrieving register values; see getAllRegisterDataForEntry()
 *		
 *	-------------------------------------------------------------------------------------------------
*/
ns_xtr::xtr_data ns_xtr::data_manager::_TRACE64FORMAT_convertToXTREntry(const x64dbg_trace_block& entry)
{
	_TRACE64FORMAT_updateAllExceptRC(entry);

	x64dbg_trace_block adjustedMO512Entry = entry;
	std::vector<REGISTERCONTEXT> affectedregs;

	if (_preserveData)	// MO512
	{
		bool isFirstEntry = xtrTrace->getTotalInstructionCount() == 0;

		// apply changes to data if not first entry
		if (!isFirstEntry)
		{
			adjustedMO512Entry.regchanges.clear();
			adjustedMO512Entry.regdata.clear();

			// want specific indices that changed. DONT FORGET ABOUT ABSORELATIVE FORM! 
			// (already dealt with here but for future reference) 
			for (_DWORD i = 0; i < entry.RegisterChanges; i++)
			{
				if (_full_entry_context.regdata[i] != entry.regdata[i])
				{
					adjustedMO512Entry.regdata.push_back(entry.regdata[i]);
					affectedregs.push_back(static_cast<REGISTERCONTEXT>(i));

				}
			}

			adjustedMO512Entry.regchanges = getAbsoRelativeForm(affectedregs);
			adjustedMO512Entry.RegisterChanges = adjustedMO512Entry.regchanges.size();
		}
		else // assuming 1st entry has full ctx; true if valid .trace data
		{
			_full_entry_context._transferRegDataFrom(entry);
		}
		_entry_cip = entry.regdata[RIP]; // full ctx so doesnt require check like below

		xtrTrace->_saveOriginalMO512(_full_entry_context);

	}
	else
	{
		affectedregs = getRCForm(adjustedMO512Entry.regchanges);
		// check that RIP does have new val.
		for (_DWORD j = 0; j < affectedregs.size(); j++)
		{
			_full_entry_context.regdata[affectedregs[j]] = adjustedMO512Entry.regdata[j];

			if (affectedregs[j] == RIP)
			{
				_entry_cip = adjustedMO512Entry.regdata[j];
			}

		}

	}

	return xtr_data(affectedregs, _entry_cip, adjustedMO512Entry);
}

/*
* Parses bytes into 'binary trace blocks' (format of x64dbg data entries)  
*	- unspecified in x64dbg documentation but THIS guarantees EVERY 512th entry will have
*	  full context. 
* 
*  converted into xtr_data format, which allows direct access to relevant data.
*/
void ns_xtr::data_manager::_trace64ToXTR()
{
	//see https://github.com/x64dbg/docs/blob/master/developers/tracefile.md 
	x64dbg_trace_block trace64_data;
	std::vector<REGISTERCONTEXT> changes;


	_TRACE64FORMAT_BYTE1_readBlockType(trace64_data); 
	_TRACE64FORMAT_BYTE2_readRegisterChanges(trace64_data);

	bool isMO512Entry = ((xtrTrace->getTotalInstructionCount()) % constants::TRACE64_ENTRY_MULTIPLE_OF_512) == 0;

	bool isSaveAllConstant = (trace64_data.RegisterChanges == constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_32BIT
							 || 
							  trace64_data.RegisterChanges == constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_64BIT);

	//x64dbg saves all reg data at very start of a trace/every 512th
	if (isMO512Entry || isSaveAllConstant)
	{
		_preserveData = true;
	}

	_TRACE64FORMAT_BYTE3_readMemoryAccesses(trace64_data);
	_TRACE64FORMAT_BYTE4_readBlockFlagsAndOpcodeSize(trace64_data);
	_TRACE64FORMAT_BYTE5_readRegisterChangePosition(trace64_data);
	_TRACE64FORMAT_QWORD6_readRegisterChangeNewData(trace64_data);
	_TRACE64FORMAT_BYTE7_readMemoryAccessFlags(trace64_data);
	_TRACE64FORMAT_QWORD8_readMemoryAccessAddresses(trace64_data);
	_TRACE64FORMAT_QWORD9_readMemoryAccessOldData(trace64_data);
	_TRACE64FORMAT_QWORD10_readMemoryAccessNewData(trace64_data);


	if (!xtrTrace->isEncounteredThreadID(trace64_data.ThreadID)) 
	{
		xtrTrace->_createThreadIDEntry(trace64_data.ThreadID);
	}
	
	xtrTrace->_addInstructionEntryForThreadID(trace64_data.ThreadID, _TRACE64FORMAT_convertToXTREntry(trace64_data));

	_preserveData = false; // reset
}



// lol
void ns_xtr::data_manager::_appendToNewFileBuf(_BYTE* srcval, _QWORD datalen)
{
	if (srcval != nullptr && datalen != 0)
	{
		for (_DWORD i = 0; i < datalen; i++) 
		{
			_newfile_databuf.push_back((*(srcval + i) & 0xFF));
		}
	}
}

void ns_xtr::data_manager::_appendToNewFileBuf(_BYTE srcval)
{
	_newfile_databuf.push_back(((srcval) & 0xFF));
}

void ns_xtr::data_manager::_appendToNewFileBuf(_WORD srcval)
{
	_newfile_databuf.push_back(((srcval) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 8) & 0xFF));
}

void ns_xtr::data_manager::_appendToNewFileBuf(_DWORD srcval)
{
	_newfile_databuf.push_back(((srcval) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 8) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 16) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 24) & 0xFF));
}

void ns_xtr::data_manager::_appendToNewFileBuf(_QWORD srcval)
{
	_newfile_databuf.push_back(((srcval) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 8) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 16) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 24) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 32) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 40) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 48) & 0xFF));
	_newfile_databuf.push_back(((srcval >> 56) & 0xFF));
}

void ns_xtr::data_manager::_clearNewFileBuf()
{
	_newfile_databuf.clear();
}


void ns_xtr::data_manager::_TRACE64FORMAT_BYTE1_readBlockType(x64dbg_trace_block& entry)
{
	entry.blocktype = _provided_file->_readByte(); // 1st field = BlockType. currently only 0 defined.
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE2_readRegisterChanges(x64dbg_trace_block& entry)
{
	entry.RegisterChanges = _provided_file->_readByte(); // Field 2: len(RegisterChangePosition) and len(RegisterChangeNewData). eg: if RegisterChanges = 4, len(RegisterChangePosition) = 4 and len(RegisterChangeNewData) = 32 [4 QWORDs]
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE3_readMemoryAccesses(x64dbg_trace_block& entry)
{
	entry.MemoryAccesses = _provided_file->_readByte(); // Field 3: len(MemoryAccessFlags)
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE4_readBlockFlagsAndOpcodeSize(x64dbg_trace_block& entry)
{
	_BYTE BlockFlagsAndOpcodeSize = _provided_file->_readByte();
	_BYTE opcode_size = 0;

	// BlockFlagsAndOpcodeSize - MSB(bit) set? Y - entry for ThreadID exists. N - next.
	if (BlockFlagsAndOpcodeSize & constants::bitstuff::X64DBG_BLOCK_THREAD_ID_MASK_SET) {

		entry.ThreadID = _provided_file->_readDataSize(constants::LEN_DWORD);
		_prev_TID = entry.ThreadID; // save entry to check
	}
	else { // MSB not set; thread ID is same

		if (!_prev_TID) // TID == 0 should never happen if valid trace64 block
		{
			std::cout << "threadID == 0 or invalid block format" << std::endl;
			exit(EXIT_FAILURE); // lol
		}

		entry.ThreadID = _prev_TID; // restore
	}

	// lower 4 bits represent length of opcode bytes
	opcode_size = BlockFlagsAndOpcodeSize & constants::bitstuff::X64DBG_BLOCK_OPCODE_LENGTH_MASK;

	for (_DWORD i = 0; i < opcode_size; i++) 
	{
		entry.opcodes.push_back(_provided_file->_readByte());
	}
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE5_readRegisterChangePosition(x64dbg_trace_block& entry)
{
	// RegisterChanges # of bytes
	for (_DWORD i = 0; i < entry.RegisterChanges; i++) 
	{
		entry.regchanges.push_back(_provided_file->_readByte());
	}
}

void ns_xtr::data_manager::_TRACE64FORMAT_QWORD6_readRegisterChangeNewData(x64dbg_trace_block& entry)
{
	// RegisterChanges # of QWORDs
	for (_DWORD i = 0; i < entry.RegisterChanges; i++) 
	{
		entry.regdata.push_back(_provided_file->_readDataSize(constants::LEN_QWORD));
	}
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE7_readMemoryAccessFlags(x64dbg_trace_block& entry)
{
	_BYTE value = 0;

	// MemoryAccessFlags array
	for (_DWORD i = 0; i < entry.MemoryAccesses; i++) 
	{
		value = _provided_file->_readByte();

		if ((value & 1) == 0) 
		{ 
			// When bit 0 is set, it indicates the memory is not changed (This could mean it is read, or it is overwritten with identical value). bit 0 not set = mem change
			entry.mem_access_entries += 1;
			entry.mem_acc_flags.push_back(true);
		}
		else 
		{
			entry.mem_acc_flags.push_back(false);
		}

	}

}

void ns_xtr::data_manager::_TRACE64FORMAT_QWORD8_readMemoryAccessAddresses(x64dbg_trace_block& entry)
{
	// MemoryAccesses # of QWORDs
	for (_DWORD i = 0; i < entry.MemoryAccesses; i++) 
	{
		entry.mem_acc_addr.push_back(_provided_file->_readDataSize(constants::LEN_QWORD));
	}
}

void ns_xtr::data_manager::_TRACE64FORMAT_QWORD9_readMemoryAccessOldData(x64dbg_trace_block& entry)
{
	// MemoryAccesses # of QWORDs
	for (_DWORD i = 0; i < entry.MemoryAccesses; i++) 
	{
		entry.mem_acc_old.push_back(_provided_file->_readDataSize(constants::LEN_QWORD));
	}
}

void ns_xtr::data_manager::_TRACE64FORMAT_QWORD10_readMemoryAccessNewData(x64dbg_trace_block& entry)
{
	// mem_access_entries # of QWORDs. DIFFERENT THAN ABOVE 2
	for (_DWORD i = 0; i < entry.mem_access_entries; i++) 
	{
		entry.mem_acc_new.push_back(_provided_file->_readDataSize(constants::LEN_QWORD));
	}
}


/*
* returns a vector containing values of registers for specified entry.
*	
*	if invalid, vector returned is full of zeros
*
* DOES NOT CARE IF YOU 'REMOVED' ENTRIES. WHAT MATTERS IS WHETHER U CALL THIS 
  BEFORE OR AFTER _TRACE64FORMAT_adjustEntryContext()

	gets values of registers for curr entry.

since trace block regdata format is RLE-compressed, need to re-calculate anytime you want 'updated' vals.

	two options: use this 
					*BEFORE* _TRACE64FORMAT_adjustEntryContext()
					  OR 
					*AFTER*

	example scenario:
		- original trace has 5000 entries.
		- filtering the trace results in 2000 entries

		if you call this fn prior to _TRACE64FORMAT_adjustEntryContext(),
		any index <= 4999 will yield valid result.

		call it after? u can only access the filtered 2000 entries

*/
std::vector<_QWORD> ns_xtr::data_manager::getAllRegisterDataForEntry(_DWORD Nth_entry)
{
	std::vector<_QWORD> full_regdata;

	_DWORD nearestMO512 = Nth_entry / constants::TRACE64_ENTRY_MULTIPLE_OF_512;
	_DWORD startingIndex = (nearestMO512 * constants::TRACE64_ENTRY_MULTIPLE_OF_512);

	bool isMultipleOf512 = ((Nth_entry) % constants::TRACE64_ENTRY_MULTIPLE_OF_512) == 0;

	bool invalid_index = false;

	if (_filterCalled)	// did we call _TRACE64FORMAT_adjustEntryContext() THEN lookup tracedata?
	{
		bool hasEntries = (entries_to_keep.size() > 0);
		bool withinBounds = (Nth_entry < entries_to_keep.size());

		// filtered data is NOT empty and valid index was specified
		if (hasEntries && withinBounds)
		{
			_DWORD originalTraceSize = xtrTrace->getTotalInstructionCount();

			if (startingIndex < entries_to_keep.size())
			{
				auto& full_context_entry = xtrTrace->getInstructionFromNthEntry(entries_to_keep[startingIndex]);

				if (isMultipleOf512)	// MO512 entry of filtered data has full context. no need to re-calculate.
				{
					return full_context_entry.getRegisterData();
				}

				auto filtered_full_context = full_context_entry._get_trace_block();

				for (_DWORD i = startingIndex + 1; i <= Nth_entry; i++)
				{
					const auto& filtered_nth_ins = xtrTrace->getNthEntry(entries_to_keep[i]);
					const auto changes = filtered_nth_ins._affected_regs;

					const auto& filtered_nth_ins_regdata = xtrTrace->_TRACE64FORMAT_getNthInstructionRegChangesData(entries_to_keep[i]);

					for (_DWORD j = 0; j < changes.size(); j++)
					{
						filtered_full_context.regdata[changes[j]] = filtered_nth_ins_regdata[j];
					}
				}

				full_regdata = filtered_full_context.regdata;
			}
			else 
			{
				invalid_index = true;
			}
		}
		else  
		{
			invalid_index = true;
		}
	}
	else // looking up entry before calling _TRACE64FORMAT_adjustEntryContext()
	{
		bool withinBounds = Nth_entry < xtrTrace->getTotalInstructionCount();

		if (withinBounds)
		{
			auto savedMO512 = xtrTrace->_getOriginalMO512(nearestMO512);

			if (isMultipleOf512) // MO512 entry of original data has full context stored separately. no need to re-calculate.
			{
				return savedMO512.regdata;
			}


			// start from 1st entry. guaranteed to have all context.
			for (_DWORD i = startingIndex + 1; i <= Nth_entry; i++)
			{
				const auto& nth_ins = xtrTrace->getNthEntry(i);
				const auto changes = nth_ins._affected_regs;

				const auto& nth_ins_regdata = xtrTrace->_TRACE64FORMAT_getNthInstructionRegChangesData(i);

				for (_DWORD j = 0; j < changes.size(); j++)
				{
					savedMO512.regdata[changes[j]] = nth_ins_regdata[j];
				}

			}
			full_regdata = savedMO512.regdata;
		}
		else
		{
			invalid_index = true;
		}
		
	}
	

	if (invalid_index)	// fill list with 0s if invalid.
	{
		FEXTBT trace_type = _provided_file->getFileExt();

		_DWORD ctx_len = (trace_type == EXT_TYPE_64) // shouldnt even get EXT_TYPE_UNK 
						?
						constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_64BIT
						:
						constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_32BIT;

		for (_DWORD i = 0; i < ctx_len; i++)
		{
			full_regdata.push_back(0);
		}
		 
	}
	
	return full_regdata;
}

/*
* returns TRUE if there was a memory READ or memory WRITE.
*----------------------------------------------
* ------ memory READ sets memtype to MEMATYPE_READ
* ------ memory WRITE sets memtype to MEMATYPE_WRITE


*/
bool ns_xtr::data_manager::doesEntryHaveMemaData(_DWORD Nth_entry)
{
	auto& nth_ins = xtrTrace->getInstructionFromNthEntry(Nth_entry);
	bool hasMemaData = nth_ins.getMemoryAccessesCount() > 0;

	return hasMemaData;
}

/*
* returns a vector of memadata object(s). CHECK MEMBER memtype FOR MORE INFO.
* ---- if memtype == MEMATYPE_WRITE, newval is valid.
* ---- if memtype == MEMATYPE_READ, newval is NOT valid.
* ---- if memtype == MEMATYPE_NOTSET, there was no mem access.
*/
std::vector<ns_xtr::memadata> ns_xtr::data_manager::getEntryMemaData(_DWORD Nth_entry)
{
	std::vector<memadata> result;

	if (doesEntryHaveMemaData(Nth_entry))
	{
		auto& nth_ins = xtrTrace->getInstructionFromNthEntry(Nth_entry);
		const std::vector<bool>& mem_acc_flags = nth_ins.getMemoryAccessesFlags();

		const auto& mem_acc_addrs = nth_ins.getMemoryAccessesAddresses();
		const auto& mem_acc_new = nth_ins.getMemoryAccessesNew();
		const auto& mem_acc_old = nth_ins.getMemoryAccessesOld();

		for (_DWORD i = 0; i < mem_acc_flags.size(); i++) 
		{
			memadata entryMemoryData; // all fields zero by default

			if (mem_acc_flags[i])
			{
				entryMemoryData.memtype = MEMATYPE_WRITE;
				entryMemoryData.new_val = mem_acc_new[i];
			}
			else
			{
				entryMemoryData.memtype = MEMATYPE_READ;
			}

			
			entryMemoryData.address = mem_acc_addrs[i];
			entryMemoryData.old_val = mem_acc_old[i];

			result.push_back(entryMemoryData);
	
		}
		
	}


	return result;
}


void ns_xtr::data_manager::_TRACE64FORMAT_getNewTraceOrder()
{
	for (_DWORD i = 0; i < xtrTrace->getTotalInstructionCount(); i++)
	{
		if (xtrTrace->doesNthEntryExist(i))
		{
			entries_to_keep.push_back(i);
		}
	}
}

/*
* this current implementation is only viable b/c the MO512 constraint was dealt with when 
* processing into xtr_data..
*/
void ns_xtr::data_manager::_TRACE64FORMAT_reLinkNewEntries()
{
	_DWORD newTraceSize = entries_to_keep.size();

	for (_DWORD i = 0; i +1 < newTraceSize; i++)
	{
		_TRACE64FORMAT_redirectValidContext(entries_to_keep[i], entries_to_keep[i + 1]);
	}
}

/*
*  Responsible for updating the 'target' (aka next_valid_index) with the result of
*  the changes caused by the last valid instruction.
*
*	param1) LAST index of element that was valid.
*	param2) CURRENT index of element that is valid.
*
*		------------------------------
*		NOTE: IMPLIES param1 != param2
*		------------------------------
*
*		0 <= M < M+1 < N
*		
*		where 
* 
*		M corresponds to last_valid_index
*		N corresponds to next_valid_index
*
*	core steps
*
*		1) confirm gap b/w specified indices
*		2) get trace_blocks of entries M, M+1, and N.
		3) Preserve current IP of N. overwrite RCs data of N with that of M + 1. restore IP of N.

*
*/
void ns_xtr::data_manager::_TRACE64FORMAT_redirectValidContext(_DWORD last_valid_index, _DWORD next_valid_index)
{
	bool srcIsValid = xtrTrace->doesNthEntryExist(last_valid_index);	// entry M
	bool dstIsValid = xtrTrace->doesNthEntryExist(next_valid_index);    // entry N

	bool notConsecutive = (next_valid_index - last_valid_index) > 1;

	if ((srcIsValid && dstIsValid)
		&&
		notConsecutive)
	{
		auto& target_entry = xtrTrace->getNthEntry(next_valid_index); // N
		auto& target_entry_ins = target_entry._ins_data;  
		auto& target_entry_ins_context = target_entry_ins._get_trace_block();

		auto target_cip_copy = target_entry_ins.getCIP();

		auto& src_result_entry = xtrTrace->getNthEntry(last_valid_index + 1); // M + 1
		auto& src_result_entry_ins = src_result_entry._ins_data;	 
		const auto& src_result_ins_context = src_result_entry_ins._get_trace_block();

		target_entry_ins_context._transferRegDataFrom(src_result_ins_context);
		target_entry._affected_regs = src_result_entry._affected_regs;
 
		for (_DWORD j = 0; j < target_entry._affected_regs.size(); j++)
		{
			if (target_entry._affected_regs[j] == RIP)
			{
				target_entry_ins_context.regdata[j] = target_cip_copy;
			}

		}
	}
}
/*

* Calls the fns that do the magic w/ RLE-compressed delta data
* 
*	MUST be called AFTER all analyses done. (only if you plan on creating a new .trace file 
*											 OR 
*											 if you plan on using entries_to_keep)
* 
*	basically 'chains' together all the valid entries and updates them accordingly.
* 
* 
*		NOTE: calling this may have effects on the tracedata. the original trace data itself
			  is actually modified since a copy is NOT made for the 'new' trace. that's why
			  you call this function AFTER filtering/doing whatever with the data.
* 
* 
*	current implementation is the most straightforward possible method to reduce likelihood of 
*	applying wrong changes. 

		TODO?: work out kinks involved from updating the "proper" way 
			 (which would achieve this in 1 go as opposed to looping entries_to_keep 3 times
			 dont think it matters tho..?)
		
			core idea [current implementation]
			---------
				1) loop through all entries from initial tracedata
				2) if valid entry, add to entries_to_keep <- list that tracks elements of new trace
				3) once done, loop through entries_to_keep and 'connect' each one (implies gap exists between the 2 elements being looked at)
				4) propagate changes resulting from the entry to _full_entry_context
					- basically gets us the register values as if we only executed the instructions
					  specified in entries_to_keep
					- by only applying appropriate changes, the effects are reflected in _full_entry_context
					  so that by the time we reach a multiple of 512 (w/ respect to new trace ordering) we'll
					  have the correct adjusted full context.
				4) if entry is a multiple of 512, make it full context (regardless of whether or not x64dbg does)
*/
void ns_xtr::data_manager::_TRACE64FORMAT_adjustEntryContext()
{
	if (!_filterCalled)
		_filterCalled = true;
	// messier but simpler
	_TRACE64FORMAT_getNewTraceOrder();

	_TRACE64FORMAT_reLinkNewEntries();

	_DWORD newTraceSize = entries_to_keep.size();

	if (newTraceSize >= 1)
	{
		bool isNewMO512 = false;
		_full_entry_context = xtrTrace->_getOriginalMO512(0);

		// i = filtered entry
		for (_DWORD i = 0; i < newTraceSize; i++)
		{
			isNewMO512 = (i % constants::TRACE64_ENTRY_MULTIPLE_OF_512) == 0;

			auto& nth_entry = xtrTrace->getNthEntry(entries_to_keep[i]);
			const auto changes = nth_entry._affected_regs;

			const auto& nth_entry_regdata = xtrTrace->_TRACE64FORMAT_getNthInstructionRegChangesData(entries_to_keep[i]);

			auto orig_cip_copy = nth_entry.getCIP();

			for (_DWORD k = 0; k < changes.size(); k++)
			{
				_full_entry_context.regdata[changes[k]] = nth_entry_regdata[k];
			}

			if (isNewMO512)	// makes ALL multiple of 512s have full data
			{
				auto& newMO512Ins = nth_entry._ins_data;

				_full_entry_context.regdata[RIP] = orig_cip_copy;
				_full_entry_context.ThreadID = newMO512Ins.getThreadID();
				_full_entry_context.opcodes = newMO512Ins.getOpcodes();
				_full_entry_context.mem_acc_addr = newMO512Ins.getMemoryAccessesAddresses();
				_full_entry_context.mem_acc_flags = newMO512Ins.getMemoryAccessesFlags();
				_full_entry_context.mem_acc_new = newMO512Ins.getMemoryAccessesNew();
				_full_entry_context.mem_acc_old = newMO512Ins.getMemoryAccessesOld();
				_full_entry_context.MemoryAccesses = newMO512Ins.getMemoryAccessesCount();
				_full_entry_context.mem_access_entries = newMO512Ins.getMemaWriteCount();
				newMO512Ins._replaceEntry(_full_entry_context);
				
			}
				
		}
	}
	else 
	{
		 
		std::cout << "all entries were removed?.. tracedata can't be empty!"
					<< "\n\tfiltered trace size: " << newTraceSize 
					<< "\n\toriginal trace size: " << xtrTrace->getTotalInstructionCount() 
					<< "\n\nExiting..." << std::endl;
		 
	}
	 
}

/* Converts xtrdata entries back into .trace64 file format.
* 
* ASSUMES ALL ANALYSES ARE DONE. ASSUMES THERE'S NO MORE CHANGES TO THE TRACEDATA TO FINALIZE THE NEW ORDERING/FORMAT.
* 
*	_TRACE64FORMAT_<data type>_write...() fns store the data (converted back into .trace64 format) in _newfile_databuf
*	and is 'reset' after each entry is committed. 
*
*/

_DWORD ns_xtr::data_manager::_TRACE64FORMAT_writeAsTrace64Entry(_DWORD Nth_Entry)
{
	_DWORD bytes_written = 0;

	if (xtrTrace->doesNthEntryExist(Nth_Entry)) 
	{
		_BYTE RegChangeCount = xtrTrace->_TRACE64FORMAT_getNthInstructionRegChangesCount(Nth_Entry);
		bool success = false;

		

		bool isMultipleOf512 = (_new_entry_count % constants::TRACE64_ENTRY_MULTIPLE_OF_512 == 0);
		bool isSaveAllConstant = (RegChangeCount == constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_64BIT
								 ||
								  RegChangeCount == constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_32BIT);

		_new_entry_count++;

		// Need to save values if at least one of the following:
			// Condition 1: (64-bit) - RegisterChangeCount == 172 (0xAC)		| (32-bit) - next time
			// Condition 2: very 1st entry OR entry is a multiple of 512.

		if (isMultipleOf512 || isSaveAllConstant)
		{
			_preserveData = true;
		}

		// does the hard work! : )
		_TRACE64FORMAT_BYTE1_writeBlockType(Nth_Entry);
		_TRACE64FORMAT_BYTE2_writeRegisterChanges(Nth_Entry);
		_TRACE64FORMAT_BYTE3_writeMemoryAccesses(Nth_Entry);
		_TRACE64FORMAT_BYTE4_writeBlockFlagsAndOpcodeSize(Nth_Entry);
		_TRACE64FORMAT_BYTE5_writeRegisterChangePosition(Nth_Entry);
		_TRACE64FORMAT_QWORD6_writeRegisterChangeNewData(Nth_Entry);
		_TRACE64FORMAT_BYTE7_writeMemoryAccessFields(Nth_Entry);

		success = WriteFile(_newfile, &_newfile_databuf[0], 
										_newfile_databuf.size(), 
										reinterpret_cast<LPDWORD>(&bytes_written), NULL);

		_preserveData = false; // reset
		_clearNewFileBuf();

	}
	return bytes_written;
}

void ns_xtr::data_manager::_TRACE64FORMAT_HEADER1_writeMagicBytes()
{
	std::string trace64_magic = "TRAC";

	_appendToNewFileBuf(reinterpret_cast<_BYTE*>(&trace64_magic[0]), trace64_magic.length());
}

void ns_xtr::data_manager::_TRACE64FORMAT_HEADER2_writeJsonData()
{
	_DWORD bloblen = _newjsonblob.length();

	// DWORD indicating size of upcoming json blob data
	_appendToNewFileBuf(reinterpret_cast<_BYTE*>(&bloblen), constants::TRACE64_FILE_JSON_BLOB_HEADER_SIZE);

	// json blob data
	_appendToNewFileBuf(reinterpret_cast<_BYTE*>(&_newjsonblob[0]), bloblen);
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE1_writeBlockType(_DWORD Nth_Entry)
{
	// start entry with 0. no other values are defined
	_appendToNewFileBuf(constants::TRACE64_ENTRY_BLOCKTYPE_ID);
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE2_writeRegisterChanges(_DWORD Nth_Entry)
{
	_BYTE RegisterChangesValue = xtrTrace->_TRACE64FORMAT_getNthInstructionRegChangesCount(Nth_Entry);
	_appendToNewFileBuf(RegisterChangesValue);
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE3_writeMemoryAccesses(_DWORD Nth_Entry)
{
	_BYTE MemoryAccessesValue = xtrTrace->_TRACE64FORMAT_getNthInstructionMemAccessesCount(Nth_Entry);
	_appendToNewFileBuf(MemoryAccessesValue);
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE4_writeBlockFlagsAndOpcodeSize(_DWORD Nth_Entry)
{
	bool setMSB = false;
	_BYTE BlockFlagsAndOpcodeSizeValue = 0;

	if (!_prev_TID) { // _prev_TID should never be 0 unless we just started to output data as trace64
		_prev_TID = xtrTrace->_TRACE64FORMAT_getNthInstructionThreadID(Nth_Entry);
		setMSB = true;
	}
	else // _prev_TID = nonzero; check if diff
	{
		_DWORD curr_TID = xtrTrace->getThreadIDForInstruction(Nth_Entry);

		if ((_prev_TID != curr_TID) || _preserveData) { // ids diff or (n*512)th entry
			_prev_TID = curr_TID;
			setMSB = true;
		}
		// otherwise, tids are same; keep default val
	}

	// get opcode bytes of curr ins
	std::vector<_BYTE>& op_bytes = xtrTrace->_TRACE64FORMAT_getNthInstructionOpcodes(Nth_Entry);

	BlockFlagsAndOpcodeSizeValue = op_bytes.size();

	if (setMSB) {// if MSB set, need write TID. else dw
		BlockFlagsAndOpcodeSizeValue |= constants::bitstuff::X64DBG_BLOCK_THREAD_ID_MASK_SET; // set MSB; entry exists so next write is a DWORD

		_appendToNewFileBuf(BlockFlagsAndOpcodeSizeValue);
		_appendToNewFileBuf(_prev_TID);	// thread val added since MSB set

	}
	else
	{
		_appendToNewFileBuf(BlockFlagsAndOpcodeSizeValue);
	}

	
	_appendToNewFileBuf(&op_bytes[0], op_bytes.size());
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE5_writeRegisterChangePosition(_DWORD Nth_Entry)
{
	const std::vector<_BYTE>& RegChangePositions = xtrTrace->_TRACE64FORMAT_getNthInstructionRegChangesIndices(Nth_Entry);

	for (_DWORD i = 0; i < RegChangePositions.size(); i++) {
		_appendToNewFileBuf(RegChangePositions[i]);
	}	

}

void ns_xtr::data_manager::_TRACE64FORMAT_QWORD6_writeRegisterChangeNewData(_DWORD Nth_Entry)
{
	const std::vector<_QWORD>& RegChangeNewData = xtrTrace->_TRACE64FORMAT_getNthInstructionRegChangesData(Nth_Entry);

	for (_DWORD i = 0; i < RegChangeNewData.size(); i++) {
		_appendToNewFileBuf(RegChangeNewData[i]);
	}
}

void ns_xtr::data_manager::_TRACE64FORMAT_BYTE7_writeMemoryAccessFields(_DWORD Nth_Entry)
{
	const std::vector<_BYTE> mem_acc_flags = xtrTrace->_TRACE64FORMAT_getNthInstructionMemAccessesFlags(Nth_Entry);

	const std::vector<_QWORD>& mem_acc_addrs = xtrTrace->_TRACE64FORMAT_getNthInstructionMemAccessesAddrs(Nth_Entry);
	const std::vector<_QWORD>& mem_acc_old = xtrTrace->_TRACE64FORMAT_getNthInstructionMemAccessesOld(Nth_Entry);
	const std::vector<_QWORD>& mem_acc_new = xtrTrace->_TRACE64FORMAT_getNthInstructionMemAccessesNew(Nth_Entry);

	// data MUST be consecutive (which is why 4 separate loops); see trace64 file format specs
	for (_DWORD i = 0; i < mem_acc_flags.size(); i++)
	{
		_appendToNewFileBuf(mem_acc_flags[i]);
	}

	for (_DWORD i = 0; i < mem_acc_addrs.size(); i++)
	{
		_appendToNewFileBuf(mem_acc_addrs[i]);
	}

	for (_DWORD i = 0; i < mem_acc_old.size(); i++)
	{
		_appendToNewFileBuf(mem_acc_old[i]);
	}

	for (_DWORD i = 0; i < mem_acc_new.size(); i++)
	{
		_appendToNewFileBuf(mem_acc_new[i]);
	}
}
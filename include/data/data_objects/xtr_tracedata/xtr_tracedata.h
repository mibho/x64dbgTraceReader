#pragma once

#include "trace_instruction.h"


namespace ns_xtr {

	struct xtr_data {

		xtr_data(std::vector<REGISTERCONTEXT> affected_regs,
					_QWORD cip,
					x64dbg_trace_block entrydata) : _ins_data(cip, entrydata), 
													_affected_regs(affected_regs)
													{};

		_QWORD getCIP()						{ return _ins_data.getCIP(); }
		_QWORD getThreadID()				{ return _ins_data.getThreadID(); }
		std::vector<_BYTE>& getOpcodes()	{ return _ins_data.getOpcodes(); }


		std::vector<REGISTERCONTEXT> _affected_regs;
		traceinstruction _ins_data;	
	};

	class xtr_tracedata {
	public:
		xtr_tracedata()
						{};
		~xtr_tracedata()
						{};

		/*
	
		TL;DR: use getTotalInstructionCount().


			getTotalInstructionCount() vs getValidInstructionCount()
			-------------------------------------------------------
			ONLY RELEVANT IF YOU'RE PLANNING TO DEOBFUSCATE A TRACE

			getTotalInstructionCount() - returns # of entries read from provided .trace file; ie, total size 

			getValidInstructionCount() - returns # of entries that remain AFTER filtering/removing unwanted ones

		*/
		_DWORD getThreadIDCount();
		_DWORD getValidInstructionCount();
		_DWORD getTotalInstructionCount();
		_DWORD getInstructionCountForThreadID(_DWORD threadID);
		bool isEncounteredThreadID(_DWORD threadID);
		
		/*-------------------------------------------------------------------------------------------------------------------
		* NOTE: IT'S CALLER'S RESPONSIBILITY TO ENSURE INDEX IS VALID FOR FUNCTIONS BELOW.
		* --------------------------------------------------------------------------------
		*/
		traceinstruction& getInstructionFromNthEntry(_DWORD Nth_entry);
		xtr_data& getNthEntry(_DWORD Nth_entry);
		umap<_DWORD, xtr_data>& getTraceEntryForThreadID(_DWORD threadID);
		/*
		*-------------------------------------------------------------------------------------------------------------------*/
		bool doesNthEntryExist(_DWORD Nth_entry);
		bool removeNthEntry(_DWORD Nth_entry);
		_DWORD getThreadIDForInstruction(_DWORD instructionID);
		std::vector<_DWORD> getAllThreadIDs();

		
		
		bool _createThreadIDEntry(_DWORD threadID);
		bool _addInstructionEntryForThreadID(_DWORD threadID, xtr_data ins_info);

		/*
		* fns solely for formatting. can ignore
		*/
		_BYTE _TRACE64FORMAT_getNthInstructionMemAccessesCount(_DWORD instructionID);
		_BYTE _TRACE64FORMAT_getNthInstructionRegChangesCount(_DWORD instructionID);

		_DWORD _TRACE64FORMAT_getNthInstructionThreadID(_DWORD instructionID);
		
		std::vector<_BYTE>& _TRACE64FORMAT_getNthInstructionOpcodes(_DWORD instructionID);
		_BYTE _TRACE64FORMAT_getNthInstructionOpcodesLen(_DWORD instructionID);
		
		std::vector<_BYTE>& _TRACE64FORMAT_getNthInstructionRegChangesIndices(_DWORD instructionID);
		std::vector<_QWORD>& _TRACE64FORMAT_getNthInstructionRegChangesData(_DWORD instructionID);
		
		std::vector<_BYTE> _TRACE64FORMAT_getNthInstructionMemAccessesFlags(_DWORD instructionID); // returns COPY, not ref (b/c need convert)
		std::vector<_QWORD>& _TRACE64FORMAT_getNthInstructionMemAccessesAddrs(_DWORD instructionID);
		std::vector<_QWORD>& _TRACE64FORMAT_getNthInstructionMemAccessesOld(_DWORD instructionID);
		std::vector<_QWORD>& _TRACE64FORMAT_getNthInstructionMemAccessesNew(_DWORD instructionID);
		
		
		void _saveOriginalMO512(const x64dbg_trace_block& fullcontext)
		{
			_originalMO512Entries.push_back(fullcontext);
		}

		x64dbg_trace_block& _getOriginalMO512(_DWORD indexAsMO512)
		{
			return _originalMO512Entries[indexAsMO512];
		}


	private:
		//umap<ThreadID, umap<execution #, xtr_data>> - could've used umap<DWORD, vector<xtr_data>> but removing elements affects original order
		umap<_DWORD, umap<_DWORD, xtr_data>> _history;
		std::vector<_DWORD> _inorderTIDs;
		std::vector<bool> _entries_exist;
		_DWORD _ins_num = 0;
		_DWORD _valid_entry_count = 0;

		// keep for convenience when all reg data required
		std::vector<x64dbg_trace_block> _originalMO512Entries;

		std::string _error_log = "";
	};
}
#include "../../../../include/data/data_objects/xtr_tracedata/xtr_tracedata.h"

using namespace ns_xtr;


_DWORD ns_xtr::xtr_tracedata::getThreadIDCount()
{
	return _history.size();
}

// most likely won't use unless analyzing trace via Capstone. returns # of entries remaining.
_DWORD ns_xtr::xtr_tracedata::getValidInstructionCount()
{
	return _valid_entry_count;
}

// returns # of instructions recorded in provided trace file
_DWORD ns_xtr::xtr_tracedata::getTotalInstructionCount()
{
	return _ins_num;
}

// return # of instructions that 'belong' to specified thread ID. returns ZERO if invalid tID.
_DWORD ns_xtr::xtr_tracedata::getInstructionCountForThreadID(_DWORD threadID)
{
	if (isEncounteredThreadID(threadID))
	{
		return _history.at(threadID).size();
	}

	return 0;
}

bool ns_xtr::xtr_tracedata::isEncounteredThreadID(_DWORD threadID)
{
	return _history.contains(threadID);
}

bool ns_xtr::xtr_tracedata::_createThreadIDEntry(_DWORD threadID)
{
	if (_history.contains(threadID))
	{
		std::cout << "threadID: " << threadID << " is already registered!" << std::endl;
		return false;
	}

	_history.insert(threadID, umap<_DWORD, xtr_data>());

	return true;
}

bool ns_xtr::xtr_tracedata::_addInstructionEntryForThreadID(_DWORD threadID, xtr_data ins_info)
{
	if (isEncounteredThreadID(threadID))
	{
		_history.at(threadID).insert(_ins_num, ins_info);

		_entries_exist.push_back(true);
		_inorderTIDs.push_back(threadID);
		
		_ins_num++;
		_valid_entry_count++;

		return true;
	}

	return false;
}

// NOTE: IT'S CALLER'S RESPONSIBILITY TO ENSURE INDEX IS VALID
xtr_data& ns_xtr::xtr_tracedata::getNthEntry(_DWORD Nth_entry)
{
	return _history.at(_inorderTIDs[Nth_entry]).at(Nth_entry);
}

// NOTE: IT'S CALLER'S RESPONSIBILITY TO ENSURE INDEX IS VALID
traceinstruction& ns_xtr::xtr_tracedata::getInstructionFromNthEntry(_DWORD Nth_entry)
{
	return getNthEntry(Nth_entry)._ins_data;
}

// NOTE: IT'S CALLER'S RESPONSIBILITY TO ENSURE INDEX IS VALID  : )
umap<_DWORD, xtr_data>& ns_xtr::xtr_tracedata::getTraceEntryForThreadID(_DWORD threadID)
{
	if (isEncounteredThreadID(threadID))
	{
		return _history.at(threadID);
	}

}

bool ns_xtr::xtr_tracedata::doesNthEntryExist(_DWORD Nth_entry)
{
	if (Nth_entry < _entries_exist.size())
	{
		return _entries_exist[Nth_entry];
	}

	return false;
}

bool ns_xtr::xtr_tracedata::removeNthEntry(_DWORD Nth_entry)
{
	if (Nth_entry < _ins_num)
	{
		if (_entries_exist[Nth_entry])
		{
			_valid_entry_count--;
			_entries_exist[Nth_entry] = false;
			return true;
		}
		
	}
	return false;
}

_DWORD ns_xtr::xtr_tracedata::getThreadIDForInstruction(_DWORD instructionID)
{
	if (instructionID < _ins_num)
	{
		return _inorderTIDs[instructionID];
	}

	return 0;
}

std::vector<_DWORD> ns_xtr::xtr_tracedata::getAllThreadIDs()
{
	std::vector<_DWORD> thread_entries;

	for (auto umap_itr = _history.begin(); umap_itr != _history.end(); ++umap_itr) 
	{
		auto tID = umap_itr->first;
		thread_entries.push_back(tID);
	}

	return thread_entries;
}

_BYTE ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionMemAccessesCount(_DWORD instructionID)
{
	return getInstructionFromNthEntry(instructionID).getMemoryAccessesCount();
}

_BYTE ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionRegChangesCount(_DWORD instructionID)
{
	return getInstructionFromNthEntry(instructionID).getRegisterChangesCount();
}

_DWORD ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionThreadID(_DWORD instructionID)
{
	return getThreadIDForInstruction(instructionID);
}

std::vector<_BYTE>& ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionOpcodes(_DWORD instructionID)
{
	return getInstructionFromNthEntry(instructionID).getOpcodes();
}

_BYTE ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionOpcodesLen(_DWORD instructionID)
{
	return getInstructionFromNthEntry(instructionID).getOpcodes().size();
}

std::vector<_BYTE>& ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionRegChangesIndices(_DWORD instructionID)
{
	traceinstruction& nth_ins = getInstructionFromNthEntry(instructionID);
	std::vector<_BYTE>& changed = nth_ins.getRegisterChanges();

	return changed;
}

std::vector<_QWORD>& ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionRegChangesData(_DWORD instructionID)
{
	traceinstruction& nth_ins = getInstructionFromNthEntry(instructionID);
	std::vector<_QWORD>& registerdata = nth_ins.getRegisterData();

	return registerdata;

}

std::vector<_BYTE> ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionMemAccessesFlags(_DWORD instructionID) 
{

	traceinstruction& nth_ins = getInstructionFromNthEntry(instructionID);
	const std::vector<bool>& mem_acc_flags = nth_ins.getMemoryAccessesFlags();

	std::vector<_BYTE> memflags;

	for (int i = 0; i < mem_acc_flags.size(); i++)
	{
		_BYTE flagval = (mem_acc_flags[i]) ? 0 : 1; // if mem changed, flagval = 0. ie, BIT 0 IS NOT SET.
		memflags.push_back(flagval);
	}

	return memflags;
}

std::vector<_QWORD>& ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionMemAccessesAddrs(_DWORD instructionID)
{
	traceinstruction& nth_ins = getInstructionFromNthEntry(instructionID);
	std::vector<_QWORD>& mem_acc_addrs = nth_ins.getMemoryAccessesAddresses();

	return mem_acc_addrs;
}

std::vector<_QWORD>& ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionMemAccessesOld(_DWORD instructionID)
{
	traceinstruction& nth_ins = getInstructionFromNthEntry(instructionID);
	std::vector<_QWORD>& mem_acc_old = nth_ins.getMemoryAccessesOld();

	return mem_acc_old;
}

std::vector<_QWORD>& ns_xtr::xtr_tracedata::_TRACE64FORMAT_getNthInstructionMemAccessesNew(_DWORD instructionID)
{
	traceinstruction& nth_ins = getInstructionFromNthEntry(instructionID);
	std::vector<_QWORD>& mem_acc_new = nth_ins.getMemoryAccessesNew();

	return mem_acc_new;
}

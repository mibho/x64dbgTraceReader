#pragma once
#include "../../data_formats.h"
#include <string>

namespace ns_xtr {

    using namespace dtypes;

    // layout of data contained in an entry from .trace64 file
    struct x64dbg_trace_block {

        // memdata corresponding to entry N is in N
        void _transferMemaDataFrom(const x64dbg_trace_block& updated)
        {
            MemoryAccesses = updated.MemoryAccesses;
            mem_acc_flags = updated.mem_acc_flags;
            mem_acc_addr = updated.mem_acc_addr;
            mem_acc_old = updated.mem_acc_old;
            mem_acc_new = updated.mem_acc_new;
            mem_access_entries = updated.mem_access_entries;
        }

        // regdata corresponding to entry N in N+1
        void _transferRegDataFrom(const x64dbg_trace_block& updated)
        {
            RegisterChanges = updated.RegisterChanges;
            regchanges = updated.regchanges;
            regdata = updated.regdata;
        }

        void _updateToFullSavedContext(const x64dbg_trace_block& updated, FEXTBT trace_type)
        {
            if (trace_type == EXT_TYPE_32)
                RegisterChanges = constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_32BIT;
            else
                RegisterChanges = constants::TRACE64_ENTRY_ALL_REGISTERS_SAVED_64BIT;

            ThreadID = updated.ThreadID;
            regchanges = updated.regchanges;
            regdata = updated.regdata;
        }
        
     
        _BYTE blocktype = 0;
        _BYTE RegisterChanges = 0;
        _BYTE MemoryAccesses = 0;
        _DWORD ThreadID = 0;
        std::vector<_BYTE> opcodes;
        std::vector<_BYTE> regchanges;
        std::vector<_QWORD> regdata;
        std::vector<bool> mem_acc_flags;
        std::vector<_QWORD> mem_acc_addr;
        std::vector<_QWORD> mem_acc_old;
        std::vector<_QWORD> mem_acc_new;
        _DWORD mem_access_entries = 0;
    };
     

    class traceinstruction {
    public:
        traceinstruction(_QWORD cip, x64dbg_trace_block entrydata)
        {
            _cip = cip;
            _entrydata = entrydata;
        };

        _QWORD getCIP()         { return _cip; };
        
        _BYTE getRegisterChangesCount()         { return _entrydata.RegisterChanges; }
        _BYTE getMemoryAccessesCount()          { return _entrydata.MemoryAccesses; }
        _QWORD getOpcodesCount()                { return _entrydata.opcodes.size(); };
        _DWORD getMemaWriteCount()              { return _entrydata.mem_access_entries; }
        _DWORD getThreadID()                    { return _entrydata.ThreadID; }

        std::vector<_BYTE>& getOpcodes()                    { return _entrydata.opcodes; }
        std::vector<_BYTE>& getRegisterChanges()            { return _entrydata.regchanges; }
        std::vector<_QWORD>& getRegisterData()              { return _entrydata.regdata; }

        std::vector<bool>& getMemoryAccessesFlags()         { return _entrydata.mem_acc_flags; }
        
        std::vector<_QWORD>& getMemoryAccessesAddresses()   { return _entrydata.mem_acc_addr; }
        std::vector<_QWORD>& getMemoryAccessesOld()         { return _entrydata.mem_acc_old; }
        std::vector<_QWORD>& getMemoryAccessesNew()         { return _entrydata.mem_acc_new; }
        


        void _replaceEntry(const x64dbg_trace_block& entry)           { _entrydata = entry; }
        x64dbg_trace_block& _get_trace_block()                        { return _entrydata; }

    
    private:
        _QWORD _cip;
        x64dbg_trace_block _entrydata;
    };
}
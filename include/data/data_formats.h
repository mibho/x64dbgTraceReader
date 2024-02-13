#pragma once
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <cstdlib>
#include <stdlib.h>
#include <iostream>

namespace ns_xtr {

	namespace dtypes 
    {
        // wow!
        #if _WIN64
            typedef uint8_t  _BYTE;
            typedef uint16_t _WORD;
            typedef uint32_t _DWORD;
            typedef uint64_t _QWORD;
        #else
            typedef uint8_t  _BYTE;
            typedef uint16_t _WORD;
            typedef uint32_t _DWORD;
            typedef uint32_t _QWORD;
        #endif
	}
	using namespace dtypes;

	namespace constants // start ::constants
    {

        constexpr char DEFAULT_OUTPUT_FILE_NAME[]                       = "processed_trace";
        constexpr _BYTE TRACE64_FILE_EXT_LENGTH                         = 0x7;
        constexpr _BYTE TRACE64_MINIMUM_FILE_EXT_LENGTH                 = 0x8;
        constexpr _BYTE NUMBER_OF_REGISTER_VALUES_TO_SAVE               = 0x1E;	// 30 (dec)

		constexpr _BYTE ZERO_BYTE     = 0x0;


        constexpr _BYTE NONE          = 0;
        constexpr _BYTE LEN_BYTE      = 1;
        constexpr _BYTE LEN_WORD      = 2;
        constexpr _BYTE LEN_DWORD     = 4;
        constexpr _BYTE LEN_QWORD     = 8;

            namespace errors // ::constants::errors
            { 

                constexpr char FAIL_CREATE_X64DBG_TRACEDATA[]               = "\tFailed to create x64dbg_tracedata object...\n";
                constexpr char FAIL_CREATE_X64DBGTRACEREADER[]              = "\tFailed to create x64dbgTraceReader object...\n";
                constexpr char FAIL_CREATE_DATA_MANAGER[]                   = "\tFailed to create data_manager object...\n";

                constexpr char FAIL_GET_VALID_INPUT_FILE_HANDLE[]           = "\tFailed to obtain handle or size of the specified input file!\n\nExiting...\n";
                constexpr char EMPTY_OR_INVALID_INPUT_FILE_SIZE[]           = "[!] - Supplied file is empty/blank or failed to detect its actual size!\n\nExiting...\n";
            }

		    namespace bitstuff // ::constants::bitstuff
            {
			    constexpr _BYTE X64DBG_BLOCK_THREAD_ID_MASK_SET			    = 0x80;		  // 128 (dec)		| 1000 0000 (binary); MSB set
			    constexpr _BYTE X64DBG_BLOCK_OPCODE_LENGTH_MASK			    = 0xF;		  // 15 (dec)		| 0000 1111 (binary)
		    }

		constexpr _DWORD TRACE64_FILE_MAGIC_BYTES_LITTLE_ENDIAN		    = 0x43415254; // 1,128,354,388 (dec)   |  T R A C = 0x54 0x52 0x41 0x43
		constexpr _BYTE  TRACE64_FILE_MAGIC_SIG_LENGTH				    = 0x4;
		constexpr _BYTE  TRACE64_FILE_JSON_BLOB_HEADER_SIZE			    = 0x4;
        constexpr _BYTE  TRACE64_FILE_MINIMUM_REQUIRED_ENTRIES          = 0x1;

		constexpr _WORD  TRACE64_ENTRY_MULTIPLE_OF_512				    = 0x200;	  // 512 (dec)
        constexpr _WORD  TRACE64_ENTRY_MIDPOINT_OF_NEXT_512             = 0x100;      // 256 (dec)
		
        constexpr _BYTE  TRACE64_ENTRY_ALL_REGISTERS_SAVED_64BIT		= 0xAC;		  // 172 (dec)
        constexpr _BYTE  TRACE64_ENTRY_ALL_REGISTERS_SAVED_32BIT        = 0xD8;		  // 216 (dec)

		constexpr _BYTE  TRACE64_ENTRY_BLOCKTYPE_ID					    = 0x0;		  // only 0 defined atm
		constexpr _BYTE  TRACE64_ENTRY_VALUE_ZERO					    = 0x0;
		constexpr _BYTE  XTR_ENTRY_ALL_REGISTERS_SAVED				    = 0x1E;		  // 30  (dec)
	
    } // end ::constants

    typedef enum FEXTBT { // File Extension Bit Type

        EXT_TYPE_UNK = 0,
        EXT_TYPE_32 = 1,
        EXT_TYPE_64 = 2

    } FEXTBT;

    typedef enum MEMATYPE { // Memory Access Type

        MEMATYPE_NOTSET = 0,
        MEMATYPE_READ   = 1,
        MEMATYPE_WRITE  = 2

    } MEMATYPE;

    typedef enum REGISTERCONTEXT {

        RAX = 0,
        RCX,    // 1
        RDX,    // 2
        RBX,    // 3
        RSP,    // 4
        RBP,    // 5
        RSI,    // 6
        RDI,    // 7
        R8,     // 8
        R9,     // 9
        R10,    // 10
        R11,    // 11
        R12,    // 12
        R13,    // 13
        R14,    // 14
        R15,    // 15
        RIP,    // 16
        RFLAGS, // 17
        GS,     // 18
        FS,     // 19
        ES,     // 20
        DS,     // 21
        CS,     // 22
        SS,     // 23
        DR0,    // 24
        DR1,    // 25
        DR2,    // 26
        DR3,    // 27
        DR6,    // 28
        DR7,    // 29

        RC30,  RC31,  RC32,  RC33,  RC34,  RC35,  RC36,  RC37,  RC38,  RC39,
        RC40,  RC41,  RC42,  RC43,  RC44,  RC45,  RC46,  RC47,  RC48,  RC49,
        RC50,  RC51,  RC52,  RC53,  RC54,  RC55,  RC56,  RC57,  RC58,  RC59,
        RC60,  RC61,  RC62,  RC63,  RC64,  RC65,  RC66,  RC67,  RC68,  RC69,
        RC70,  RC71,  RC72,  RC73,  RC74,  RC75,  RC76,  RC77,  RC78,  RC79,
        RC80,  RC81,  RC82,  RC83,  RC84,  RC85,  RC86,  RC87,  RC88,  RC89,
        RC90,  RC91,  RC92,  RC93,  RC94,  RC95,  RC96,  RC97,  RC98,  RC99,
        RC100, RC101, RC102, RC103, RC104, RC105, RC106, RC107, RC108, RC109,
        RC110, RC111, RC112, RC113, RC114, RC115, RC116, RC117, RC118, RC119,
        RC120, RC121, RC122, RC123, RC124, RC125, RC126, RC127, RC128, RC129,
        RC130, RC131, RC132, RC133, RC134, RC135, RC136, RC137, RC138, RC139,
        RC140, RC141, RC142, RC143, RC144, RC145, RC146, RC147, RC148, RC149,
        RC150, RC151, RC152, RC153, RC154, RC155, RC156, RC157, RC158, RC159,
        RC160, RC161, RC162, RC163, RC164, RC165, RC166, RC167, RC168, RC169,
        RC170, RC171 
        // 30-119 = 80
        // 120 = x87fpu
        // 121 = MxCSR
        // 
    } REGISTERCONTEXT;


    // labelmapper
    template <typename T, typename Y>
    class umap {

        // on top for visibility; default private
        typedef typename std::unordered_map<T, Y>::iterator umap_iterator;
        typedef typename std::unordered_map<T, Y>::const_iterator umap_const_iterator;

    public:
        umap() {};
        umap(const std::unordered_map<T, Y>& umapobj) : _umap(umapobj) 
                                                        {};
        Y& at(T& argtype1)                   { return _umap.at(argtype1); };
        const Y& at(const T& argtype1) const { return _umap.at(argtype1); };

        const uint64_t size() const          { return _umap.size(); };
        bool empty()                         { return _umap.empty(); };
        
        //see: https://stackoverflow.com/questions/60459011/contains-is-not-a-member-of-stdmap-msvc
        bool contains(const T& target) const { return (_umap.count(target) >= 1); };// requires C++20 standard { return _umap.contains(target); }

        void clear()                         { _umap.clear(); };
        
        umap_iterator begin() noexcept       { return _umap.begin(); };
        umap_iterator end() noexcept         { return _umap.end(); };

        void insert(T index, Y output)       { _umap.insert(std::make_pair(index, output)); };
        bool insert(std::pair<T, Y> input)   { auto tmp = _umap.insert(input);  return tmp.second; };

        umap_iterator find(const T& target)                 { return _umap.find(target); };
        umap_const_iterator find(const T& target) const     { return _umap.find(target); };
        
        const uint64_t erase(const T& target)               { return _umap.erase(target); };
        umap_iterator erase(umap_iterator pos)              { return _umap.erase(pos); };
        umap_const_iterator erase(umap_const_iterator pos)  { return _umap.erase(pos); };
        umap_const_iterator erase(umap_const_iterator first, 
                                  umap_const_iterator last) { return _umap.erase(first, last); };
      
    private:
        std::unordered_map<T, Y> _umap;
    };

}
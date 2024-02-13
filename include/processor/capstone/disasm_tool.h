#pragma once

#include "capstone_engine.h"
namespace ns_xtr {

	class disasm_tool {
	public:
		disasm_tool();
		~disasm_tool();

		void disasm(uint64_t ins_addr, const _BYTE* code, size_t code_size, size_t limit);


		
	private:
		capstone_engine* _capstone = nullptr;
	};

}
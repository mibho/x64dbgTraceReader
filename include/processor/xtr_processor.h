#pragma once

#include "capstone/disasm_tool.h"
#include "regex/regex_tool.h"

namespace ns_xtr {

	class XTRProcessor : public disasm_tool, public regex_tool {
	public:
		XTRProcessor() : disasm_tool(), regex_tool() {};
		~XTRProcessor();
	private:

	};
}
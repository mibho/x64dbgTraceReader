#pragma once

#include "data/data_manager.h"
#include "processor/xtr_processor.h"

namespace ns_xtr {


	class x64dbgTraceReader : protected data_manager {
	public:
		x64dbgTraceReader(const char* file_name) 
		try : data_manager(file_name) 
		{
			if (!_failed)
			{
				if (xtr_processor == nullptr)
					xtr_processor = new XTRProcessor();

				std::cout << "x64dbgTraceReader constructor called w/ data_manager successfully initialized!" << std::endl;
			}
			else
			{
				_cleanupIfNeededAndExit();
			}
			

		}
		catch (...) 
		{
			std::cout << constants::errors::FAIL_CREATE_X64DBGTRACEREADER << std::endl;
			_cleanupIfNeededAndExit();
		
		}


		void sampleTest1();

		~x64dbgTraceReader();
	private:
		XTRProcessor* xtr_processor = nullptr; // init iff x64dbgTraceReader constructor successful

		void _cleanup();
		void _cleanupIfNeededAndExit();
	};
}
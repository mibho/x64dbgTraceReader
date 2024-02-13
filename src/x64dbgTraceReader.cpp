#include "../include/x64dbgTraceReader.h"


void ns_xtr::x64dbgTraceReader::sampleTest1()
{
	if (xtr_processor != nullptr) {

		processFile();

		for (int i = 0; i < xtrTrace->getTotalInstructionCount(); i++)
		{
			if (xtrTrace->doesNthEntryExist(i))
			{
				if (doesEntryHaveMemaData(i))
				{
					auto memdata = getEntryMemaData(i);
					
					for (_DWORD j = 0; j < memdata.size(); j++)
					{
						if (memdata[j].memtype == MEMATYPE_READ)
						{
							std::cout << "Memory read for entry #" << i << ": ADDRESS = " << memdata[j].address << " VALUE = " << memdata[j].old_val << std::endl;
						}
						else if (memdata[j].memtype == MEMATYPE_WRITE)
						{
							std::cout << "Memory write for entry #" << i << ": ADDRESS = " << memdata[j].address
								<< "\n\tOLD VALUE = " << memdata[j].old_val
								<< "\n\tNEW VALUE = " << memdata[j].new_val << std::endl;
						}
					}

					
				}

			}
		}
	}
}

ns_xtr::x64dbgTraceReader::~x64dbgTraceReader()
{
	_cleanup();
	std::cout << "x64dbgTraceReader destructor called!" << std::endl;
}

void ns_xtr::x64dbgTraceReader::_cleanup()
{
	if (xtr_processor != nullptr) {
		delete xtr_processor;
		xtr_processor = nullptr;
	}
	_cleanup_data_manager();
	std::cout << "x64dbgTraceReader cleanup called!" << std::endl;
}

void ns_xtr::x64dbgTraceReader::_cleanupIfNeededAndExit()
{
	_cleanup();
	exit(EXIT_FAILURE); // lol
}

#include <iostream>

#include "../include/x64dbgTraceReader.h"
#include <Windows.h>



int main(int argc, char* argv[]) 
{
	if (argc != 2)
	{
		std::cout << "usage: x64dbgTraceReader <INSERT TRACEFILE NAME>" << std::endl;
		return 1;
	}
 
	const char* file_name = argv[1];
	ns_xtr::x64dbgTraceReader xtr = ns_xtr::x64dbgTraceReader(file_name); 
	 
	xtr.sampleTest1();
	std::cout << "finished!" << std::endl;
	return 0;
}
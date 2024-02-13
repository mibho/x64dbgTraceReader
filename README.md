# x64dbgTraceReader

### unofficial x64dbg trace parser/editor


[x64dbg](https://github.com/x64dbg/x64dbg) has a built-in tracer that makes it easy to record and view execution traces. Although this is valuable for the insight it can provide
regarding a program's behavior, its utility is often limited since it's difficult to process the data (very obscure and annoying format). To make the most of x64dbg's tracing feature, this tool aims to address 
that burden while maintaining integrity of the trace's data.

#### NOTE 1: At the moment, this only supports .trace64 files. 32-bit hasn't been tested but should work with minor changes to `REGISTERCONTEXT` (to-do).
#### NOTE 2: if someone could share a .trace64 file with multiple thread IDs logged that'd be much appreciated!

## Index

[Features](#features)

[Dependencies/Requirements](#dependenciesrequirements)

[Project structure](#project-structure)

[How to use](#how-to-use)

[Relevant objects/functions](#relevant-objectsfunctions)

[Example usage](#example-usage)
  - [Getting register values for each entry](#getting-register-values-for-each-entry)
  - [Getting memory access data for each entry](#getting-memory-access-data-for-each-entry)
  - [Filtering entries to produce a .trace file that only contains relevant instructions](#filtering-entries-to-produce-a-trace-file-that-only-contains-relevant-instructions)

[Filtered .trace example (see included .zip file)]()


[Potential to-do stuff/cool ideas?](#potential-to-docool-ideas)

## Features

  - **handles required formatting (processes [RLE-compressed](https://lemire.me/blog/2009/11/24/run-length-encoding-part-i/) data stored in 'absorelative' (absolute + relative) indexed format while preserving full context of every entry that's a multiple of 512 (MO512))**
  - **edit and save .trace files without invalidating format.**
  - **retain register data and memory included in original trace.**
  - **easily access all register values for any entry. (0xAC (172) and 0xD8 (216) registers for .trace64 and .trace32, respectively).**
  - **easily retrieve memory access data and memory values. (if applicable; not all entries will involve memory.)**
 

## Dependencies/Requirements
compiled on [Visual Studio 2022](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=Community&channel=Release&version=VS2022&source=VSLandingPage&passive=false&cid=2030) (unsure if VS2022 is **required** or if it's compatible with older versions. used default settings)

Requires [Capstone](https://github.com/capstone-engine/capstone/). (v5.0.1; can be easily removed so that Capstone isnt needed )
  - if you have everything set up, but you're getting `CS_ERR_HANDLE` or `CS_ERR_CSH`, make sure you have the big .dll (like 6.7 mb?) and
    not the dud one (< 200 kb) in the same directory.

Also make sure you are using a somewhat recent build of [x64dbg](https://github.com/x64dbg/x64dbg) to record traces. 
  - builds from or after **Jul. 14 2023** seem to be fine.
  - versions earlier than **Sep. 14 2020** are wonky and do not consistently record/update writes to memory.


## Project structure

As shown below, x64dbgTraceReader consists of two parts (2nd part, xtr_processor, is optional). The main idea is that `data_manager` takes care of the formatting so that the trace data is easily accessible (converts x64dbg trace entries into xtr_data entries). `xtr_processor` is where you would 
have your functions for analysis (after implementing stuff for Capstone and regex). Then you'd go through each entry of xtrTrace (object containing trace data) in `x64dbgTraceReader` and process them w/ `xtr_processor` while keeping track of entries you want to deal with   

![image](https://github.com/mibho/x64dbgTraceReader/assets/86342821/1dafd6f4-6e39-4ab7-ab2b-769ec9d5ab0e)

  - xtr_processor is empty for now but there's a basic implementation of Capstone functions so it's still possible to perform basic analyses.
  - **It's recommended that you define and use your own implementation of functions since the one provided is incomplete.**

### Brief overview of files
    
`trace64data_buf.h` - buffer that contains the input file bytes and reads variable sized data as needed.

`x64dbg_tracedata.h` - basic properties (eg: file name, size, handle) regarding the provided .trace file. Fills the buffer with data.

`trace_instruction.h` - [x64dbg binary trace block](https://help.x64dbg.com/en/latest/developers/tracefile.html) w/ CIP 'extracted' from absolute + relative indexed data. "hides" processing 
involved w/ the original x64dbg blocks.

`xtr_tracedata.h` - implementation of `xtr_data` and the container that handles these entries. 

`data_manager.h` - most of the processing can be found here. responsible for converting original x64dbg data into easier format and back into valid .trace file after applying changes.


 
## How to use

- Read below along with the [relevant objects/functions](#relevant-objectsfunctions) section and [Example usage](#example-usage)

### Please keep the following in mind
  1) **if you have no interest in the implementation details, you can ignore everything that isn't *camelCased* EXCEPT:**

```C
 // applies changes, re-arranges data, and restores format so that it's possible 
 // to access values of the filtered trace.
 void _TRACE64FORMAT_adjustEntryContext(); 
```

  2) **The x64dbg [binary trace block format](https://help.x64dbg.com/en/latest/developers/tracefile.html) is designed such that the *CURRENT* entry contains the effects/results of the *PREVIOUS* instruction.**

     - in other words, the **states of registers BEFORE executing the current instruction are recorded. (example below)**
  
  3) **if you remove dead/junk instructions, the new trace may have different values for those associated registers when compared to the original trace. (this is b/c the changes from the instruction(s) aren't propagated, which is reflected in the end result)**

suppose we have the following traces (other data such as RIP omitted):

```
T1 (original)                           | T2 (modified/filtered)
initial state                           | initial state
rdi = 6, rax = 1234                     | rdi = 6, rax = 1234
                                        |
1) sub rdi, 5                           | 1) sub rdi, 5
                                        |
2) mov rax, 2     <- dead/junk          | 2) xor eax, eax
                                        |
3) xor eax, eax                         | 3) add eax, 1
                                        |
4) add eax, 1                           |
                                        |
------------------------------------------------------------------------------------ 
1) getAllRegisterDataForEntry(1)        | 1) getAllRegisterDataForEntry(1)
                                        |
  rdi = 6,  rax = 1234                  |    rdi = 6,  rax = 1234
                                        |
2) getAllRegisterDataForEntry(2)        | 2) getAllRegisterDataForEntry(2)
                                        |
  rdi = 1,  rax = 1234                  |    rdi = 1,  rax = 1234
                                        |
3) getAllRegisterDataForEntry(3)        | 3) getAllRegisterDataForEntry(3)
                                        |
  rdi = 1,  rax = 2                     |    rdi = 1,  rax = 0
                                        |
4) getAllRegisterDataForEntry(4)        |
                                        |
  rdi = 1,  rax = 0                     |  

```

For the most part, the core steps can be summarized as follows:

  1) call processFile() 
  2) loop through all of the trace's entries from the start
  3) check if current entry exists (situational)
  4) get data; do w/e needed
  5) if you removed entries and want to apply changes, call _TRACE64FORMAT_adjustEntryContext();
  6) call xtrToTrace64File('provide some name') or if you'd like to output data in a different form, implement your own function

(if you need more data such as # of operands, instruction as a string, that's what Capstone in xtr_processor is for)

## Relevant objects/functions

```C++
  data_formats.h

  typedef enum MEMATYPE // Memory Access Type
  { 
      MEMATYPE_NOTSET = 0,
      MEMATYPE_READ   = 1,
      MEMATYPE_WRITE  = 2

  } MEMATYPE;

```
  - **MUST CHECK MEMATYPE field of memadata** if calling getEntryMemaData(). some entries will not have memory access data because memory isn't referenced.
      - `MEMATYPE_NOTSET` means no memadata: **address, old_val, and new_val are all invalid**
      - `MEMATYPE_READ` means: **address and old_val are valid. new_val is NOT**
      - `MEMATYPE_WRITE` means: **address, old_val, and new_val contain valid data**

```C++
  data_manager

    struct memadata
    {
        MEMATYPE memtype = MEMATYPE_NOTSET;
        _QWORD address = 0;
        _QWORD old_val = 0;
        _QWORD new_val = 0;  // MUST check MEMATYPE! 
    };

    - xtr_tracedata xtrTrace
    - std::vector<_DWORD> entries_to_keep; // represents indices to use for filtered trace

    bool processFile();  // sets up data. ONLY CALL IT ONCE in the entire project BEFORE attempting to look at data
    bool xtrToTrace64File(const char* filename); // optionally creates valid .trace64 file  w/ just the filtered entries.
    bool doesEntryHaveMemaData(_DWORD Nth_entry);
    std::vector<memadata> getEntryMemaData(_DWORD Nth_entry);	// CALLER IS RESPONSIBLE FOR ENSURING ENTRY HAS DATA.
    std::vector<_QWORD> getAllRegisterDataForEntry(_DWORD Nth_entry);
    void _TRACE64FORMAT_adjustEntryContext(); // deals w/ everything needed ONLY CALL IT ONCE IN THE ENTIRE PROJECT AFTER handling data

```

```C
  xtr_tracedata

  struct xtr_data {
      xtr_data(std::vector<REGISTERCONTEXT> affected_regs,
                                               _QWORD cip,
                             x64dbg_trace_block entrydata) : _ins_data(cip, entrydata),
                                                             _affected_regs(affected_regs)
                                                        {};

      _QWORD getCIP()                   { return _ins_data.getCIP(); }
      _QWORD getThreadID()              { return _ins_data.getThreadID(); }
      std::vector<_BYTE>& getOpcodes()  { return _ins_data.getOpcodes(); }

      std::vector<REGISTERCONTEXT> _affected_regs;
      traceinstruction _ins_data;

  };

  _DWORD getThreadIDCount();
  // most likely won't need if not planning to analyze trace
  _DWORD getValidInstructionCount(); // # of entries you didn't call removeNthEntry on

  _DWORD getTotalInstructionCount();

  _DWORD getInstructionCountForThreadID(_DWORD threadID);
  bool isEncounteredThreadID(_DWORD threadID);
  /*-------------------------------------------------------------------------------------------------------------------
  * NOTE: IT'S CALLER'S RESPONSIBILITY TO ENSURE INDEX IS VALID FOR FUNCTIONS BELOW.
  * --------------------------------------------------------------------------------
  */
  xtr_data& getNthEntry(_DWORD Nth_entry);

  // if you don't plan on changing anything 'internal', you will most likely NOT use these.
  traceinstruction& getInstructionFromNthEntry(_DWORD Nth_entry);
  // if the recorded .trace file includes multiple threads, use this to access the instructions that pertain to just the specified thread.
  umap<_DWORD, xtr_data>& getTraceEntryForThreadID(_DWORD threadID);
  /*
  *-------------------------------------------------------------------------------------------------------------------*/
  bool doesNthEntryExist(_DWORD Nth_entry);
  bool removeNthEntry(_DWORD Nth_entry);
  _DWORD getThreadIDForInstruction(_DWORD instructionID);
  std::vector<_DWORD> getAllThreadIDs();
```




## Example usage
```C++
void ns_xtr::x64dbgTraceReader::sampleTest()
{
    // technically useless if u have no plans on analyzing trace via Capstone.
    if(xtr_processor != nullptr)
    {
        // check if .trace file format then parse data so we can use xtrTrace
        processFile();
        ...
        // loop through all entries
        for (int i = 0; i < xtrTrace->getTotalInstructionCount(); i++)
        {
            // deal w/ data here

        }
        ...
        // magic that handles updating RLE-compressed vals
        _TRACE64FORMAT_adjustEntryContext();

        // creates a new .trace file 
        xtrToTrace64File("test2.trace64");
        
  
    }
}
```

### Getting register values for each entry

> x64dbgTraceReader.cpp
```C++
// go through each entry and get a vector of QWORDs (0xAC or 172 elements for .trace64) that 
// represent the current values of the registers. outputs the value of RIP and RAX.
void ns_xtr::x64dbgTraceReader::test()
{
    if (xtr_processor != nullptr)
    {
        processFile();

        for (int i = 0; i < xtrTrace->getTotalInstructionCount(); i++)
        {
            auto data = getAllRegisterDataForEntry(i);
            std::cout << "RIP: " << data[RIP] << " RAX: " << data[RAX] << std::endl;
        }
    }
}
```
  - use `REGISTERCONTEXT` (refer to data_formats.h) values to get the corresponding data. `REGISTERCONTEXT` is typedefd so it's convenient to access register data: just enter the name of the register you'd like.


### Getting memory access data for each entry

> x64dbgTraceReader.cpp
```C++
// go through each entry and
//     1) confirm entry wasn't filtered out.
//     2) confirm entry has memory access data.
//     3) if it does, get a copy of the data containing mem. acc data.
//     4) confirm the type of memory access: MEMATYPE_READ or MEMATYPE_WRITE?
//     5) if it's MEMATYPE_READ, DO NOT USE THE new_val FIELD. only old_val is valid.
//        if it's MEMATYPE_WRITE, both new_val and old_val are valid.
// outputs (if it exists) the address along with its recorded values.
void ns_xtr::x64dbgTraceReader::test()
{
    if (xtr_processor != nullptr)
    {
        processFile();

        for (int i = 0; i < xtrTrace->getTotalInstructionCount(); i++)
        {
            if (xtrTrace-> doesNthEntryExist(i))
            {
                if (doesEntryHaveMemaData(i))
                {
                    // NOTE: if entry doesn't have data for memory access, an empty vector is returned.
                    auto memdata = getEntryMemadata(i);

                    for (int j = 0; j < memdata.size(); j++)
                    {
                        // if it's MEMATYPE_READ, DO NOT USE THE new_val FIELD. only old_val is valid.
                        if (memdata[j].memtype == MEMATYPE_READ)
                        {
                            std::cout << "Memory read for entry #" << i << ": ADDRESS = " << memdata[j].address
                                      << "\nVALUE = " << memdata[j].old_val << std::endl;
                        }

                        // if it's MEMATYPE_WRITE, both new_val and old_val are valid.
                        else if(memdata[j].memtype == MEMATYPE_WRITE)
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

```


### Filtering entries to produce a .trace file that only contains relevant instructions

> x64dbgTraceReader.cpp
```C++
// loops through entire trace and removes instructions that don't fall under a specified range.
// recalculate proper values and re-order data into "filtered.trace64" file.
void ns_xtr::x64dbgTraceReader::test()
{
    if (xtr_processor != nullptr)
    {
        processFile();

        for (int i = 0; i < xtrTrace->getTotalInstructionCount(); i++)
        {
            if (xtrTrace->doesNthEntryExist(i))
            {
                auto& entry = xtrTrace->getNthEntry(i);
                auto rip = entry.getCIP();

                bool withinRange = ((rip >= 0x7ff75dc90000) && (rip <= 0x7ff75dc9a000));

                if (!withinRange)
                {
                    xtrTrace->removeNthEntry(i);
                }

            }
        }

        _TRACE64FORMAT_adjustEntryContext();
        // creates a new .trace file 
        xtrToTrace64File("filtered.trace64");
    }
}
```

then just create a `x64dbgTraceReader` object and call your function in main

> main.cpp

```C++
int main(int argc, char* argv[]) 
{
    if (argc != 2)
    {
        std::cout << "usage: x64dbgTraceReader <INSERT TRACEFILE NAME>" << std::endl;
        return 1;
    }
    const char* file_name = argv[1];
    ns_xtr::x64dbgTraceReader xtr = ns_xtr::x64dbgTraceReader(file_name);
    xtr.test();

    return 0;	
}
```

## Filtered .trace file example (see included .zip file)

The original recorded .trace file 
  -  (vmpunpack.trace64; 40 mb)

along with the analyzed one
  - (deobfuscated.trace64; 23 mb)

can be found in the attachments above. 

(it's not as cool w/o being able to explore the trace using x64dbg's UI so I recommend you check it out :))

## Potential to-do/cool ideas?

  - **make tool not a console app?..**
  - **implement regex/capstone stuff**
  - **Preserve API data from a trace session for later viewing** 
    - (need to see how viable this is). regardless, a potential workaround would be to:
        1) find intermodular calls for all modules and save the data as a .csv file (already a feature in x64dbg : ) )
        2) use regex to parse the strings and associate the labels w/ their respective addresses. (assumes .pdb symbols were already applied)
      
  - **Incorporate [LLVM](https://llvm.org/)/[Triton](https://github.com/JonathanSalwan/Triton) to do something cool like [this](https://github.com/JonathanSalwan/VMProtect-devirtualization)**
      
  - **Add features to x64dbg's trace UI or build upon [this tool](https://github.com/teemu-l/execution-trace-viewer)**

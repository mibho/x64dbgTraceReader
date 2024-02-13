#pragma once

#include <capstone/capstone.h>
#include "../../data/data_formats.h"

namespace ns_xtr {



	class capstone_engine {
	public:
		capstone_engine() 
		{
			init_capstone();
		};
		capstone_engine(cs_mode hw_mode) { init_capstone(hw_mode); }
		~capstone_engine();

		// default setting = x86-64 

		uint64_t getCIP() const;
		uint8_t getNumberOfOperands() const;

		cs_x86_op& getNthOperand(uint8_t nth_operand) const;
		uint8_t getNthOperandSize(uint8_t nth_operand) const;
		cs_ac_type getNthOperandAccessType(uint8_t nth_operand) const;	// cs_ac_type
		x86_op_type getNthOperandTypeID(uint8_t nth_operand) const;	// x86_op_type
		x86_reg getNthOperandRegisterID(uint8_t nth_operand) const;
		int64_t getNthOperandImmValue(uint8_t nth_operand) const;
		
		x86_op_mem& getNthOperandMemData(uint8_t nth_operand) const;
		x86_reg getNthOperandMemSegment(uint8_t nth_operand) const;
		x86_reg getNthOperandMemBase(uint8_t nth_operand);
		x86_reg getNthOperandMemIndex(uint8_t nth_operand);
		int getNthOperandMemIndexScale(uint8_t nth_operand);
		int64_t getNthOperandMemDisplacement(uint8_t nth_operand);

		
		x86_insn getInstructionID() const;
		std::vector<uint8_t> getAssociatedGroups();
		uint8_t getNumberOfAssociatedGroups() const;

		uint64_t getFLAGSeffects() const;

		uint8_t getNumberOfRegisterReads() const;
		uint8_t getNumberOfRegisterWrites() const;
		std::vector<uint16_t> getAllRegisterReadsByCurrentIns();
		std::vector<uint16_t> getAllRegisterWritesByCurrentIns();

		uint8_t getModRMbyte() const; // https://stackoverflow.com/questions/15511482/x64-instruction-encoding-and-the-modrm-byte
		uint16_t getInstructionSize() const;
		std::vector<uint8_t> getOpcodeBytes();
		

		bool hasPrefix();
		uint8_t getGroup1Prefix();
		uint8_t getGroup2Prefix();
		uint8_t getGroup3Prefix();
		uint8_t getGroup4Prefix(); 


		void disasm(uint64_t ins_addr, const _BYTE* code, size_t code_size, size_t limit = constants::NONE);

		void _printDetails(uint64_t ins_addr, const _BYTE* code, size_t code_size, size_t limit);

		bool _requiresFlags(); // is this ins dependent on valid eflags values?

		bool involvesFlags(); // any ins that read/writes to flags 
		
		bool onlyModifiesFlags();

		bool involvesStack(); // push/pop, pushfq/popfq, call, ret
		bool isJmpType();
		bool isJmpIns();
		bool isCallType();
		bool isRetType();
		bool isInterruptType();
		bool isInterruptRetType();
		bool isPrivilegedType();
		bool isConditionalType();
		bool isArithmeticOperator();
		bool isBitwiseOperator();
		bool isBitShiftOperator();

		bool hasExplicitEffect();
		bool hasImplicitEffect();
		
		bool isFlagAffected(uint8_t flag);



	private:
		csh _capstone_handle;
		cs_insn* _instructions = nullptr;
		size_t _ins_count = 0;
		bool _open_success = false;

		void _csdata_reset();
		size_t _disassemble(uint64_t ins_addr, const _BYTE* code, size_t code_size, size_t limit);


		// see constructor
		void init_capstone(cs_mode hw_mode = CS_MODE_64);
		void set_capstone_option(cs_opt_type option, cs_opt_value enabled);

		// see destructor
		void cleanup();


	};


}

/*
*
*
*
*	x86_insn enum vals
*	------------------
*		X86_INS_ADC,	// 6
*		X86_INS_ADCX,	// 7
*		X86_INS_ADD,	// 8
*		X86_INS_AND,	// 24
*		X86_INS_BOUND,	// 53
*		X86_INS_BSF,	// 54
*		X86_INS_BSR,	// 55
*		X86_INS_BSWAP,	// 56
*		X86_INS_BT,		// 57
*		X86_INS_BTC,	// 58
*		X86_INS_BTR,	// 59
*		X86_INS_BTS,	// 60
*		X86_INS_CALL,	// 62
*		X86_INS_CBW,	// 63
*		X86_INS_CDQ,	// 64
*		X86_INS_CDQE,	// 65
*		X86_INS_CLC,	// 68
*	X86_INS_CLD,	// 69
*	X86_INS_CLI,	// 74
*	X86_INS_CMC,	// 79
*	X86_INS_CMOVA,	// 80
*	X86_INS_CMOVAE,	// 81
*	X86_INS_CMOVB,	// 82
*	X86_INS_CMOVBE,	// 83
*	X86_INS_CMOVE,	// 86
*	X86_INS_CMOVG,	// 88
*	X86_INS_CMOVGE,	// 89
*	X86_INS_CMOVL,	// 90
*	X86_INS_CMOVLE,	// 91
*	X86_INS_CMOVNE,	// 94
*	X86_INS_CMOVNO,	// 96
*	X86_INS_CMOVNP,	// 97
*	X86_INS_CMOVNS,	// 100
*	X86_INS_CMOVO,	// 101
*	X86_INS_CMOVP,	// 102
*	X86_INS_CMOVS,	// 103
*	X86_INS_CMP,	// 104
*	X86_INS_CMPXCHG16B, // 113
*	X86_INS_CMPXCHG,	// 114
*	X86_INS_CMPXCHG8B,	// 115
*	X86_INS_CPUID,	// 123
*	X86_INS_CQO,	// 124
*	X86_INS_CWD,	// 142
*	X86_INS_CWDE,	// 143
*	X86_INS_DEC,	// 147
*	X86_INS_DIV,	// 148
*	X86_INS_ENTER,	// 166
*	X86_INS_LCALL,	// 170
*	X86_INS_LJMP,	// 171
*	X86_INS_JMP,	// 172
*	X86_INS_IDIV,	// 226
*	X86_INS_IMUL,	// 228
*	X86_INS_INC,	// 230
*	X86_INS_INT,	// 238
*	X86_INS_INT1,	// 239
*	X86_INS_INT3,	// 240
*	X86_INS_IRET,	// 248
*	X86_INS_IRETD,	// 249
*	X86_INS_IRETQ,	// 250
*	X86_INS_JAE,	// 254
*	X86_INS_JA,		// 255
*	X86_INS_JBE,	// 256
*	X86_INS_JB,		// 257
*	X86_INS_JCXZ,	// 258
*	X86_INS_JECXZ,	// 259
*	X86_INS_JE,		// 260
*	X86_INS_JGE,	// 261
*	X86_INS_JG,		// 262
*	X86_INS_JLE,	// 263
*	X86_INS_JL,		// 264
*	X86_INS_JNE,	// 265
*	X86_INS_JNO,	// 266
*	X86_INS_JNP,	// 267
*	X86_INS_JNS,	// 268
*	X86_INS_JO,		// 269
*	X86_INS_JP,		// 270
*	X86_INS_JRCXZ,	// 271
*	X86_INS_JS,		// 272
*	X86_INS_LAHF,	// 324
*	X86_INS_LEA,	// 332
*	X86_INS_LEAVE,	// 333
*	X86_INS_LFENCE,	// 335
*	X86_INS_LOCK,	// 343
*	X86_INS_LODSB,	// 344
*	X86_INS_LODSD,	// 345
*	X86_INS_LODSQ,	// 346
*	X86_INS_LODSW,	// 347
*	X86_INS_LOOP,	// 348
*	X86_INS_LOOPE,	// 349
*	X86_INS_LOOPNE,	// 350
*	X86_INS_RETF,	// 351
*	X86_INS_RETFQ,	// 352
*	X86_INS_MOV,	// 460
*	X86_INS_MOVABS,	// 461
*	X86_INS_MOVBE,	// 464
*	X86_INS_MOVSB,	// 485
*	X86_INS_MOVSD,	// 486
*	X86_INS_MOVSQ,	// 489
*	X86_INS_MOVSS,	// 490
*	X86_INS_MOVSW,	// 491
*	X86_INS_MOVSX,	// 492
*	X86_INS_MOVSXD,	// 493
*	X86_INS_MOVZX,	// 496
*	X86_INS_MUL,	// 498
*	X86_INS_NEG,	// 509
*	X86_INS_NOP,	// 510
*	X86_INS_NOT,	// 511
*	X86_INS_OR,		// 512
*	X86_INS_POP,	// 585
*	X86_INS_POPF,	// 589
*	X86_INS_POPFD,	// 590
*	X86_INS_POPFQ,	// 591
*	X86_INS_PREFETCH,	// 592
*	X86_INS_PREFETCHT0,	// 594
*	X86_INS_PREFETCHT1,	// 595
*	X86_INS_PREFETCHT2,	// 596
*	X86_INS_PREFETCHW,	// 597
*	X86_INS_PUSH,	// 609
*	X86_INS_PUSHAW,	// 610
*	X86_INS_PUSHAL,	// 611
*	X86_INS_PUSHF,	// 612
*	X86_INS_PUSHFD,	// 613
*	X86_INS_PUSHFQ,	// 614
*	X86_INS_RCL,	// 615
*	X86_INS_RCR,	// 618
*	X86_INS_RDRAND,	// 625
*	X86_INS_RDSEED,	// 626
*	X86_INS_RDTSC,	// 629
*	X86_INS_REPNE,	// 631
*	X86_INS_REP,	// 632
*	X86_INS_RET,	// 633
*	X86_INS_REX64,	// 634
*	X86_INS_ROL,	// 635
*	X86_INS_ROR,	// 636
*	X86_INS_SAHF,	// 646
*	X86_INS_SAL,	// 647
*	X86_INS_SAR,	// 649
*	X86_INS_SCASB,	// 653
*	X86_INS_SCASD,	// 654
*	X86_INS_SCASQ,	// 655
*	X86_INS_SCASW,	// 656
*	X86_INS_SETAE,	// 657
*	X86_INS_SETA,	// 658
*	X86_INS_SETBE,	// 659
*	X86_INS_SETB,	// 660
*	X86_INS_SETE,	// 661
*	X86_INS_SETGE,	// 662
*	X86_INS_SETG,	// 663
*	X86_INS_SETLE,	// 664
*	X86_INS_SETL,	// 665
*	X86_INS_SETNE,	// 666
*	X86_INS_SETNO,	// 667
*	X86_INS_SETNP,	// 668
*	X86_INS_SETNS,	// 669
*	X86_INS_SETO,	// 670
*	X86_INS_SETP,	// 671
*	X86_INS_SFENCE,	// 674
*	X86_INS_SHL,	// 683
*	X86_INS_SHLD,	// 684
*	X86_INS_SHR,	// 686
*	X86_INS_SHRD,	// 687
*	X86_INS_STC,	// 703
*	X86_INS_STD,	// 704
*	X86_INS_STI,	// 706
*	X86_INS_STOSB,	// 708
*	X86_INS_STOSD,	// 709
*	X86_INS_STOSQ,	// 710
*	X86_INS_STOSW,	// 711
*	X86_INS_SUB,	// 715
*	X86_INS_SYSCALL, // 727
*	X86_INS_TEST,	// 734
*	X86_INS_UD0,	// 746
*	X86_INS_UD1,	// 747
*	X86_INS_UD2,	// 748
*	X86_INS_XADD,	// 1491
*	X86_INS_XBEGIN,	// 1492
*	X86_INS_XCHG,	// 1493
*	X86_INS_XGETBV,	// 1501
*	X86_INS_XOR,	// 1503
*/
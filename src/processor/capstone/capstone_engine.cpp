#include "../../../include/processor/capstone/capstone_engine.h"

using namespace ns_xtr;



 uint8_t ns_xtr::capstone_engine::getNumberOfOperands() const
{
	if (_instructions != nullptr) {
		std::cout << "op count: " << static_cast<int>(_instructions->detail->x86.op_count) << std::endl;

		return _instructions->detail->x86.op_count;
	}

	return 0;
}

 x86_op_type ns_xtr::capstone_engine::getNthOperandTypeID(uint8_t nth_operand) const
{
	if (_instructions != nullptr) {
		std::cout << "op type: {}\n" << _instructions->detail->x86.operands[nth_operand].type << std::endl;
		return _instructions->detail->x86.operands[nth_operand].type;
	}

	return X86_OP_INVALID;
}

 // NOTE: IT'S CALLER'S RESPONSIBILITY TO ENSURE INDEX IS VALID
cs_x86_op& ns_xtr::capstone_engine::getNthOperand(uint8_t nth_operand) const
{
	if (_instructions != nullptr) 
	{
		return _instructions->detail->x86.operands[nth_operand];
	}
}

x86_reg ns_xtr::capstone_engine::getNthOperandRegisterID(uint8_t nth_operand) const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.operands[nth_operand].reg;
	}
	return X86_REG_INVALID;
}

int64_t ns_xtr::capstone_engine::getNthOperandImmValue(uint8_t nth_operand) const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.operands[nth_operand].imm;
	}

	return 0;
}

// NOTE: IT'S CALLER'S RESPONSIBILITY TO ENSURE INDEX IS VALID
x86_op_mem& ns_xtr::capstone_engine::getNthOperandMemData(uint8_t nth_operand) const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.operands[nth_operand].mem;
	}
}

x86_reg ns_xtr::capstone_engine::getNthOperandMemSegment(uint8_t nth_operand) const
{
	if (_instructions != nullptr)
	{
		const x86_op_mem& memdata = getNthOperandMemData(nth_operand);
		return memdata.segment;
	}

	return X86_REG_INVALID;
}

x86_reg ns_xtr::capstone_engine::getNthOperandMemBase(uint8_t nth_operand)
{
	if (_instructions != nullptr)
	{
		const x86_op_mem& memdata = getNthOperandMemData(nth_operand);
		return memdata.base;
	}

	return X86_REG_INVALID;
}

x86_reg ns_xtr::capstone_engine::getNthOperandMemIndex(uint8_t nth_operand)
{
	if (_instructions != nullptr)
	{
		const x86_op_mem& memdata = getNthOperandMemData(nth_operand);
		return memdata.index;
	}

	return X86_REG_INVALID;
}

int ns_xtr::capstone_engine::getNthOperandMemIndexScale(uint8_t nth_operand)
{
	if (_instructions != nullptr)
	{
		const x86_op_mem& memdata = getNthOperandMemData(nth_operand);
		return memdata.scale;
	}

	return X86_REG_INVALID;
}

int64_t ns_xtr::capstone_engine::getNthOperandMemDisplacement(uint8_t nth_operand)
{
	if (_instructions != nullptr)
	{
		const x86_op_mem& memdata = getNthOperandMemData(nth_operand);
		return memdata.disp;
	}

	return X86_REG_INVALID;
}



uint8_t ns_xtr::capstone_engine::getNthOperandSize(uint8_t nth_operand) const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.operands[nth_operand].size;
	}

	return 0;
}

cs_ac_type ns_xtr::capstone_engine::getNthOperandAccessType(uint8_t nth_operand) const
{
	if (_instructions != nullptr)
	{
		return static_cast<cs_ac_type>(_instructions->detail->x86.operands[nth_operand].access);
	}

	return CS_AC_INVALID;
}

x86_insn ns_xtr::capstone_engine::getInstructionID() const
{
	if (_instructions != nullptr)
	{
		// uint32 as x86_insn enum val
		return static_cast<x86_insn>(_instructions->id);
	}

	return X86_INS_INVALID;
}

std::vector<uint8_t> ns_xtr::capstone_engine::getAssociatedGroups()
{
	std::vector<uint8_t> associated_group;

	if (_instructions != nullptr)
	{
		for (int i = 0; i < _instructions->detail->groups_count; i++)
		{
			associated_group.push_back(_instructions->detail->groups[i]);
		}
	}

	return associated_group;
}

uint8_t ns_xtr::capstone_engine::getNumberOfAssociatedGroups() const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->groups_count;
	}

	return 0;
}


uint64_t ns_xtr::capstone_engine::getFLAGSeffects() const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.eflags;
	}

	return 0;
}

uint8_t ns_xtr::capstone_engine::getNumberOfRegisterReads() const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->regs_read_count;
	}

	return 0;
}

uint8_t ns_xtr::capstone_engine::getNumberOfRegisterWrites() const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->regs_write_count;
	}

	return 0;
}

std::vector<uint16_t> ns_xtr::capstone_engine::getAllRegisterReadsByCurrentIns()
{
	std::vector<uint16_t> registers_read;

	if (_instructions != nullptr)
	{
		for (int i = 0; i < _instructions->detail->regs_read_count; i++)
		{
			registers_read.push_back(_instructions->detail->regs_read[i]);
		}
	}

	return registers_read;
}

std::vector<uint16_t> ns_xtr::capstone_engine::getAllRegisterWritesByCurrentIns()
{
	std::vector<uint16_t> registers_write;

	if (_instructions != nullptr)
	{
		for (int i = 0; i < _instructions->detail->regs_write_count; i++)
		{
			registers_write.push_back(_instructions->detail->regs_write[i]);
		}
	}

	return registers_write;
}

uint8_t ns_xtr::capstone_engine::getModRMbyte() const
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.modrm;
	}

	return 0;
}

uint16_t ns_xtr::capstone_engine::getInstructionSize() const
{
	if (_instructions != nullptr)
	{
		return _instructions->size;
	}

	return 0;
}

std::vector<uint8_t> ns_xtr::capstone_engine::getOpcodeBytes()
{
	std::vector<uint8_t> opcodes;

	if (_instructions != nullptr)
	{
		for (int i = 0; i < _instructions->size; i++)
		{
			opcodes.push_back(_instructions->bytes[i]);
		}
	}

	return opcodes;
}

uint64_t ns_xtr::capstone_engine::getCIP() const
{
	if (_instructions != nullptr)
	{
		return _instructions->address;
	}

	return 0;
}

bool ns_xtr::capstone_engine::hasPrefix()
{
	if (_instructions != nullptr)
	{
		return (*reinterpret_cast<_DWORD*>(_instructions->detail->x86.prefix) > 0);
	}

	return false;
}

uint8_t ns_xtr::capstone_engine::getGroup1Prefix()
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.prefix[0];
	}

	return 0;
}

uint8_t ns_xtr::capstone_engine::getGroup2Prefix()
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.prefix[1];
	}

	return 0;
}

uint8_t ns_xtr::capstone_engine::getGroup3Prefix()
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.prefix[2];
	}

	return 0;
}
uint8_t ns_xtr::capstone_engine::getGroup4Prefix()
{
	if (_instructions != nullptr)
	{
		return _instructions->detail->x86.prefix[3];
	}

	return 0;
}

void ns_xtr::capstone_engine::disasm(uint64_t ins_addr, const _BYTE* code, size_t code_size, size_t limit)
{
	_ins_count = _disassemble(ins_addr, code, code_size, limit);

	std::cout << "errno: " << cs_errno(_capstone_handle) << std::endl;
	std::cout << "ins count: " << _ins_count << std::endl;
	if (_instructions == nullptr) {
		std::cout << "instructions is nullptr" << std::endl;
	}
	else {
		std::cout << "access: " << static_cast<int>(_instructions->detail->x86.operands->access) << std::endl;
		std::cout << "instr: " << _instructions->mnemonic << " " << _instructions->op_str << std::endl;
		//getOperandCount();
		//getNthOperandTypeID();
	}
	_csdata_reset();
}

void ns_xtr::capstone_engine::_printDetails(uint64_t ins_addr, const _BYTE* code, size_t code_size, size_t limit)
{
	_ins_count = _disassemble(ins_addr, code, code_size, limit);

	std::cout << "errno: " << cs_errno(_capstone_handle) << std::endl;
	std::cout << "ins count: " << _ins_count << std::endl;
	if (_instructions == nullptr) {
		std::cout << "instructions is nullptr" << std::endl;
	}
	else {

		std::cout << "ins ID: " << _instructions->id << std::endl;
		printf("ins EIP: %llx\n", _instructions->address);

		std::cout << "size: " << _instructions->size << " bytes: ";
		for (int i = 0; i < _instructions->size; i++)
		{
			printf("%x ", _instructions->bytes[i]);
		}
	
		std::cout << "\n" << std::endl;
		std::cout << "Operand count: " << static_cast<int>(_instructions->detail->x86.op_count) << std::endl;

		if (_instructions->detail->x86.op_count) {

			for (int i = 0; i < _instructions->detail->x86.op_count; i++) {
				std::cout << "Access info: " << static_cast<int>(_instructions->detail->x86.operands[i].access) << std::endl;

				switch (_instructions->detail->x86.operands[i].access) {
				case CS_AC_INVALID:
					std::cout << "\tCS_AC_INVALID.  (CS_AC_TYPE)(0)" << std::endl;
					break;
				case CS_AC_READ:
					std::cout << "\tCS_AC_READ.  (CS_AC_TYPE)(1)" << std::endl;
					break;
				case CS_AC_WRITE:
					std::cout << "\tCS_AC_WRITE.  (CS_AC_TYPE)(2)" << std::endl;
					break;
				case (CS_AC_READ | CS_AC_WRITE):
					std::cout << "\tCS_AC_READ_WRITE.  (CS_AC_TYPE)(3)" << std::endl;
					break;
				default:
					std::cout << "this is not supposed to print" << std::endl;
					break;

				}

				std::cout << "Operand size (if applicable): " << static_cast<int>(_instructions->detail->x86.operands[i].size) << std::endl;
				std::cout << "type # of operand: " << (_instructions->detail->x86.operands[i].type) << std::endl;

				switch (_instructions->detail->x86.operands[i].type) {
				case X86_OP_INVALID:
					std::cout << "\tX86_OP_INVALID (X86_OP_TYPE)(0)" << std::endl;
					break;
				case X86_OP_REG:
					std::cout << "\tX86_OP_REG (X86_OP_TYPE)(1)" << std::endl;
					break;
				case X86_OP_IMM:
					std::cout << "\tX86_OP_IMM (X86_OP_TYPE)(2)" << std::endl;
					break;
				case X86_OP_MEM:
					std::cout << "\tX86_OP_MEM (X86_OP_TYPE)(3)" << std::endl;
					break;
				default:
					std::cout << "this is not supposed to print" << std::endl;
					break;


				}
			}
		}
		
		
		std::cout << "instr: " << _instructions->mnemonic << " " << _instructions->op_str << std::endl;

		std::cout << "# of implicit registers read by ins: " << static_cast<int>(_instructions->detail->regs_read_count) <<  std::endl;

		if (_instructions->detail->regs_read_count) {

			for (int i = 0; i < _instructions->detail->regs_read_count; i++)
				std::cout << "reg read: " << static_cast<int>(_instructions->detail->regs_read[i]) << std::endl;
		}

		std::cout << "# of implicit registers modified by ins: " << static_cast<int>(_instructions->detail->regs_write_count) << std::endl;

		if (_instructions->detail->regs_write_count) {

			for (int i = 0; i < _instructions->detail->regs_write_count; i++)
				std::cout << "reg write: " << static_cast<int>(_instructions->detail->regs_write[i]) << std::endl;
		}

		std::cout << "# of groups this ins belongs to: " << static_cast<int>(_instructions->detail->groups_count) << std::endl;

		if (_instructions->detail->groups_count) {

			for (int i = 0; i < _instructions->detail->groups_count; i++)
				std::cout << "group: " << static_cast<int>(_instructions->detail->groups[i]) << std::endl;
		}
		


		//for (int i = 0; i < 59; i++)
		//{
		//	std::cout << "\t\tflag anded by macro " << i << " aka " << (1ULL << i) << " yields: " << (_instructions->detail->x86.eflags & (1ULL << i)) << std::endl;
		//}
		/*
		std::cout << "EFLAGS: " << (_instructions->detail->x86.eflags) << std::endl;
		std::cout << "\tanded: " << (_instructions->detail->x86.eflags & (X86_EFLAGS_SET_ZF)) << std::endl;
		std::cout << "\t\tflags tested? all TESTs : " << (_instructions->detail->x86.eflags & (X86_EFLAGS_TEST_AF |X86_EFLAGS_TEST_CF| X86_EFLAGS_TEST_DF | X86_EFLAGS_TEST_OF | X86_EFLAGS_TEST_PF | X86_EFLAGS_TEST_SF | X86_EFLAGS_TEST_ZF )) << std::endl;
		std::cout << "\n\t\tflags tested? all MODIFYs : " << (_instructions->detail->x86.eflags & (X86_EFLAGS_MODIFY_AF | X86_EFLAGS_MODIFY_CF | X86_EFLAGS_MODIFY_DF | X86_EFLAGS_MODIFY_OF | X86_EFLAGS_MODIFY_PF | X86_EFLAGS_MODIFY_SF | X86_EFLAGS_MODIFY_ZF)) << std::endl;
		std::cout << "\n\t\tflags tested? all RESETs : " << (_instructions->detail->x86.eflags & (X86_EFLAGS_RESET_AF | X86_EFLAGS_RESET_CF | X86_EFLAGS_RESET_DF | X86_EFLAGS_RESET_OF | X86_EFLAGS_RESET_PF | X86_EFLAGS_RESET_SF | X86_EFLAGS_RESET_ZF)) << std::endl;
		std::cout << "\n\t\tflags tested? all SETs : " << (_instructions->detail->x86.eflags & (X86_EFLAGS_SET_AF | X86_EFLAGS_SET_CF | X86_EFLAGS_SET_DF | X86_EFLAGS_SET_OF | X86_EFLAGS_SET_PF | X86_EFLAGS_SET_SF | X86_EFLAGS_SET_ZF)) << std::endl;
		*/
		



		for (int i = 0; i < 4; i++)
		{
			std::cout << "\t\tprefix[" << i << "] " << static_cast<int>(_instructions->detail->x86.prefix[i]) << std::endl;
		}
	
		std::cout << "casted prefix to dword: " << *reinterpret_cast<_DWORD*>(_instructions->detail->x86.prefix) << std::endl;
		for (int i = 0; i < 4; i++)
		{
			std::cout << "\t\t\topcode[" << i << "] " << static_cast<int>(_instructions->detail->x86.opcode[i]) << std::endl;
		}

	 
		std::cout << "\t\trex: " << static_cast<int>(_instructions->detail->x86.rex) << std::endl;
		std::cout << "\t\taddr_size: " << static_cast<int>(_instructions->detail->x86.addr_size) << std::endl;
		std::cout << "\t\tmodrm: " << static_cast<int>(_instructions->detail->x86.modrm) << std::endl;
		std::cout << "\t\tsib: " << static_cast<int>(_instructions->detail->x86.sib) << std::endl;
		std::cout << "\t\tdisp: " << _instructions->detail->x86.disp << std::endl;
		std::cout << "\t\tsib_index: " << static_cast<int>(_instructions->detail->x86.sib_index) << std::endl;
		std::cout << "\t\tsib_scale: " << static_cast<int>(_instructions->detail->x86.sib_scale) << std::endl;
		std::cout << "\t\tsib_base: " << static_cast<int>(_instructions->detail->x86.sib_base) << std::endl;

		std::cout << "\t\t\tmodrm_offset: " << static_cast<int>(_instructions->detail->x86.encoding.modrm_offset) << std::endl;
		std::cout << "\t\t\tdisp_offset: " << static_cast<int>(_instructions->detail->x86.encoding.disp_offset) << std::endl;
		std::cout << "\t\t\tdisp_size: " << static_cast<int>(_instructions->detail->x86.encoding.disp_size) << std::endl;
		std::cout << "\t\t\timm_offset: " << static_cast<int>(_instructions->detail->x86.encoding.imm_offset) << std::endl;
		std::cout << "\t\t\timm_size: " << static_cast<int>(_instructions->detail->x86.encoding.imm_size) << std::endl;
		
		std::cout << "\t\t\timm_size: " << static_cast<int>(_instructions->size) << std::endl;

		std::cout << "-----------" << std::endl << std::endl;
	}
	_csdata_reset();

	/*
	* doesInsAffectAF(ins_to_eval){
	* 
	* ret ins_to_eval.eflags & (X86_EFLAGS_MODIFY_AF | X86_EFLAGS_RESET_AF | X86_EFLAGS_SET_AF) -- 
	* }
	* 
	* markEIPAsLeader
	* 
	*/
}

// does current ins require valid FLAGS val to determine result?
/*
*	CMOVcc
*   Jcc
*	SETcc
*	PUSHF/POPF
*	LAHF	LOOPE	LOOPNE
*/	
bool ns_xtr::capstone_engine::_requiresFlags()
{
	if ((_instructions->id >= X86_INS_CMOVA && _instructions->id <= X86_INS_CMOVS)    // (ID >= 80 [CMOVA] and ID <= 103 [CMOVS] )
		||																	   
		(_instructions->id >= X86_INS_JAE && _instructions->id <= X86_INS_JS)		  // (ID  >= 254 [JAE] and ID <= 272 [JS])
		|| 
		(_instructions->id >= X86_INS_POPF && _instructions->id <= X86_INS_POPFQ)     // (ID >= 589 [POPF] and ID <= 591 [POPFQ] )
		||
		(_instructions->id >= X86_INS_PUSHF && _instructions->id <= X86_INS_PUSHFQ)   // (ID >= 612 [PUSHF] and ID <= 614 [PUSHFQ] )
		||
		(_instructions->id >= X86_INS_SETAE && _instructions->id <= X86_INS_SETS)	  // (ID >= 657 [SETAE] and ID <= 673 [SETS])
		||
		(_instructions->id == X86_INS_LAHF || _instructions->id == X86_INS_LOOPE || _instructions->id == X86_INS_LOOPNE)
		)
	{
		return true;
	}

	return false;
}

/*
*	popfq/pushfq 
*	lahf
*	sahf
*	CLC/CMC/STC
*	CLD/STD
*	CLI/STI
*	test
*	cmp
*	
*/
bool ns_xtr::capstone_engine::involvesFlags()
{
	if (_requiresFlags() || onlyModifiesFlags() || isArithmeticOperator() || isBitwiseOperator()|| isBitShiftOperator())
	{
		return true;
	}
	return false;
}

bool ns_xtr::capstone_engine::onlyModifiesFlags()
{
	if ((_instructions->id == X86_INS_CLC) || (_instructions->id == X86_INS_STC) ||
		(_instructions->id == X86_INS_CLD) || (_instructions->id == X86_INS_STD) ||
		(_instructions->id == X86_INS_CLI) || (_instructions->id == X86_INS_STI) ||
		(_instructions->id == X86_INS_CMC) || (_instructions->id == X86_INS_SAHF)
		||
		(_instructions->id == X86_INS_CMP  || _instructions->id == X86_INS_TEST) ||
		(_instructions->id >= X86_INS_POPF && _instructions->id <= X86_INS_POPFQ)
		||
		(_instructions->id >= X86_INS_BT   && _instructions->id <= X86_INS_BTS))
		
	{
		return true;
	}
	return false;
}

// clear any csh resources (if applicable) and free handle
void ns_xtr::capstone_engine::cleanup()
{
	_csdata_reset();

	if (_open_success)
		cs_close(&_capstone_handle);
}

ns_xtr::capstone_engine::~capstone_engine()
{
#ifdef DEBUG_ON

#endif

	cleanup();
}

void ns_xtr::capstone_engine::_csdata_reset()
{
	if (_open_success) 
	{
		if (_instructions != nullptr) 
		{
			if (_ins_count > 0) 
			{
				cs_free(_instructions, _ins_count);
				_ins_count = 0;
			}

			_instructions = nullptr;
		}

	}
}

size_t ns_xtr::capstone_engine::_disassemble(uint64_t ins_addr, const _BYTE* code, size_t code_size, size_t limit = 0)
{
	return cs_disasm(_capstone_handle, reinterpret_cast<const uint8_t*>(code), code_size, ins_addr, limit, &_instructions);
}

void ns_xtr::capstone_engine::init_capstone(cs_mode hw_mode)
{
	// cs_open returns CS_ERR_OK on success; handle successfully initialized if _open_success == CS_ERR_OK.
	_open_success = cs_open(CS_ARCH_X86, hw_mode, &_capstone_handle) == CS_ERR_OK ? true : false;

	std::cout << "CSH errno: " << cs_strerror(cs_errno(_capstone_handle)) << std::endl;
	if (!_open_success) {

		auto cs_error = cs_errno(_capstone_handle);
		std::cout << "[!] - Error setting up Capstone: " << cs_strerror(cs_error) << std::endl << "Exiting..." << std::endl;
		return;
	}

	set_capstone_option(CS_OPT_DETAIL, CS_OPT_ON);

}

void ns_xtr::capstone_engine::set_capstone_option(cs_opt_type option, cs_opt_value enabled)
{
	if (!_open_success)
		return;

	switch (option) {
	case CS_OPT_DETAIL:
		cs_option(_capstone_handle, CS_OPT_DETAIL, enabled);
		break;
	case CS_OPT_SKIPDATA:
		cs_option(_capstone_handle, CS_OPT_SKIPDATA, enabled);
		break;
	default:

		break;
	}
}

/*
*	ENTER
*	LEAVE
*	push and its variations
*	pop ' '  ^
*	call
*	ret
*	
*/
bool ns_xtr::capstone_engine::involvesStack()
{
	if ((_instructions->id >= X86_INS_POP  && _instructions->id <= X86_INS_POPFQ)     // (ID >= 589 [POPF] and ID <= 591 [POPFQ] )
		||
		(_instructions->id >= X86_INS_PUSH && _instructions->id <= X86_INS_PUSHFQ)  // (ID >= 612 [PUSHF] and ID <= 614 [PUSHFQ] )
		||
		((_instructions->id == X86_INS_ENTER || _instructions->id == X86_INS_LEAVE || // (ID == 166 [ENTER] OR ID == 333 [LEAVE])
		  _instructions->id == X86_INS_CALL  || _instructions->id == X86_INS_RET)))   // (ID == 62 [CALL] OR ID == 633 [RET]
	{
		return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isJmpIns()
{
	if ((_instructions->id >= X86_INS_JAE  && _instructions->id <= X86_INS_JS)   // (ID >= 254 [JAE] and ID <= 272 [JS] )
		||																	     
		(_instructions->id == X86_INS_LJMP || _instructions->id == X86_INS_JMP)) // (ID == 171 [LJMP] or ID == 172 [JMP])
	{
		return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isJmpType()
{
	for (int i = 0; i < 8; i++)
	{
		if (_instructions->detail->groups[i] == CS_GRP_JUMP)
			return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isCallType()
{
	for (int i = 0; i < 8; i++)
	{
		if (_instructions->detail->groups[i] == CS_GRP_CALL)
			return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isRetType()
{
	for (int i = 0; i < 8; i++)
	{
		if (_instructions->detail->groups[i] == CS_GRP_RET)
			return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isInterruptType()
{
	for (int i = 0; i < 8; i++)
	{
		if (_instructions->detail->groups[i] == CS_GRP_INT)
			return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isInterruptRetType()
{
	for (int i = 0; i < 8; i++)
	{
		if (_instructions->detail->groups[i] == CS_GRP_IRET)
			return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isPrivilegedType()
{
	for (int i = 0; i < 8; i++)
	{
		if (_instructions->detail->groups[i] == CS_GRP_PRIVILEGE)
			return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isConditionalType()
{
	for (int i = 0; i < 8; i++)
	{
		if (_instructions->detail->groups[i] == CS_GRP_BRANCH_RELATIVE)
			return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isArithmeticOperator()
{
	if ((_instructions->id >= X86_INS_ADC && _instructions->id <= X86_INS_ADD)   // (ID >= 6 [ADC] and ID <= 8 [ADD] )
		||
		(_instructions->id == X86_INS_DEC  || _instructions->id == X86_INS_INC) ||
		 _instructions->id == X86_INS_DIV  || _instructions->id == X86_INS_MUL  ||
		 _instructions->id == X86_INS_IDIV || _instructions->id == X86_INS_IMUL ||
		 _instructions->id == X86_INS_SBB  || _instructions->id == X86_INS_SUB) 
	{
		return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isBitwiseOperator()
{
	if ((_instructions->id == X86_INS_AND || _instructions->id == X86_INS_OR  ||
		 _instructions->id == X86_INS_NOT || _instructions->id == X86_INS_NEG ||
		 _instructions->id == X86_INS_XOR))
	{
		return true;
	}

	return false;
}

bool ns_xtr::capstone_engine::isBitShiftOperator()
{
	if ((_instructions->id == X86_INS_SAL || _instructions->id == X86_INS_SAR  ||
		 _instructions->id == X86_INS_SHL || _instructions->id == X86_INS_SHLD ||
		 _instructions->id == X86_INS_SHR || _instructions->id == X86_INS_SHRD ||
		 _instructions->id == X86_INS_RCL || _instructions->id == X86_INS_RCR  ||
		 _instructions->id == X86_INS_ROL || _instructions->id == X86_INS_ROR))
	{
		return true;
	}

	return false;
}

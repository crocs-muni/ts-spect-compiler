/******************************************************************************
*
* SPECT Compiler
* Copyright (C) 2022-present Tropic Square
*
* @todo: License
*
* @author Ondrej Ille, <ondrej.ille@tropicsquare.com>
* @date 19.9.2022
*
*****************************************************************************/

#include "CpuModel.h"
#include "InstructionI.h"
#include "InstructionFactory.h"
#include "Symbol.h"


spect::InstructionI::InstructionI(std::string mnemonic, uint32_t opcode, uint32_t func, int op_mask,
                                  CpuGpr op1, CpuGpr op2, uint16_t immediate, bool r31_dep, bool c_time) :
    Instruction(mnemonic, InstructionType::I, opcode, func, op_mask, r31_dep, c_time),
    op1_(op1),
    op2_(op2),
    immediate_(immediate)
{}

spect::Symbol* spect::InstructionI::Relocate()
{
    if (s_immediate_) {
        if (s_immediate_->resolved_) {
            immediate_ = s_immediate_->val_;
        } else {
            return s_immediate_;
        }
    }
    return nullptr;
}

uint32_t spect::InstructionI::Assemble()
{
    return (((immediate_     & IENC_IMMEDIATE_MASK) << IENC_IMMEDIATE_OFFSET)   |
            ((TO_INT(op2_)   & IENC_OP_MASK)        << IENC_OP2_OFFSET)         |
            ((TO_INT(op1_)   & IENC_OP_MASK)        << IENC_OP1_OFFSET)         |
            ((func_          & IENC_FUNC_MASK)      << IENC_FUNC_OFFSET)        |
            ((opcode_        & IENC_OPCODE_MASK)    << IENC_OPCODE_OFFSET)      |
            ((TO_INT(itype_) & IENC_TYPE_MASK)      << IENC_TYPE_OFFSET)
           );
}


spect::Instruction* spect::InstructionI::DisAssemble(uint32_t wrd)
{
    uint32_t immediate = (wrd >> IENC_IMMEDIATE_OFFSET) & IENC_IMMEDIATE_MASK;
    uint32_t op1       = (wrd >> IENC_OP1_OFFSET)       & IENC_OP_MASK;
    uint32_t op2       = (wrd >> IENC_OP2_OFFSET)       & IENC_OP_MASK;
    uint32_t func      = (wrd >> IENC_FUNC_OFFSET)      & IENC_FUNC_MASK;
    uint32_t opcode    = (wrd >> IENC_OPCODE_OFFSET)    & IENC_OPCODE_MASK;
    uint32_t itype     = (wrd >> IENC_TYPE_OFFSET)      & IENC_TYPE_MASK;

    spect::Instruction *instr = spect::InstructionFactory::
                                GetInstruction(INSTR_ENCODE(func, opcode, itype));
    spect::InstructionI *cln = nullptr;
    if (instr) {
        cln = (spect::InstructionI*) instr->Clone();
        cln->op1_ = TO_CPU_GPR(op1);
        cln->op2_ = TO_CPU_GPR(op2);
        cln->immediate_ = immediate;
    }
    return cln;
}

void spect::InstructionI::Dump(std::ostream& os)
{
    for (int i = 2; i >= 0; i--)
        if (op_mask_ & (1 << i)) {
            if (i == 2)
                os << op1_;
            else if (i == 1)
                os << "," << op2_;
            else
                os << "," << immediate_;
        }
}

bool spect::InstructionI::Execute()
{
    model_->DebugInfo(VERBOSITY_MEDIUM, "Inputs before execution:");

    if (op_mask_ & 0x2) {
        std::stringstream ss;
        ss << "    " << op2_ << ": " << std::hex << "0x" << model_->GetGpr(TO_INT(op2_));
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    if (op_mask_ & 0x1) {
        std::stringstream ss;
        ss << "    " << "Immediate:" << std::hex << "0x" << immediate_;
        model_->DebugInfo(VERBOSITY_MEDIUM, ss.str().c_str());
    }

    return true;
}
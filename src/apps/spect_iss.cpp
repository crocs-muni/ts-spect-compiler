/**************************************************************************************************
**
**
** TODO: License
**
** Author: Ondrej Ille
**************************************************************************************************/

#include "OptionParser.h"

#include "CpuSimulator.h"
#include "Compiler.h"
#include "CpuModel.h"
#include "CpuProgram.h"
#include "HexHandler.h"


enum  optionIndex {
    UNKNOWN,
    HELP,
    PROGRAM,
    FIRST_ADDR,
    START_PC,
    PARITY,
    CMD_FILE,
    SHELL,
    INSTRUCTION_MEM_HEX,
    CONST_ROM_HEX,
    DATA_RAM_IN_HEX,
    DATA_RAM_OUT_HEX,
    GRV_HEX,
    MAX_INSTR_CNT,
    DUMP_CONTEXT,
    LOAD_CONTEXT,
};

const option::Descriptor usage[] =
{
    {UNKNOWN,               0,  ""  ,    ""                     ,option::Arg::None,         "USAGE: spect_compiler [options]\n\n" "Options:" },
    {HELP,                  0,  "h" ,    "help"                 ,option::Arg::None,         "  --help                       Print usage and exit." },
    {PROGRAM,               0,  ""  ,    "program"              ,option::Arg::Optional,     "  --program=<s-file>           Program (unassembled) to be compiled and loaded to Instruction memory.\n"},
    {FIRST_ADDR,            0,  ""  ,    "first-address"        ,option::Arg::Optional,     "  --first-address=<addr>       Address to place first instruction from first compiled file. Use this "
                                                                                                                           "option only when loading program via '--program' switch. Option is ignored"
                                                                                                                           "when loading program from HEX file.\n" },
    {START_PC,              0,  ""  ,    "start-pc"             ,option::Arg::Optional,     "  --start-pc=<addr>            Address of first instruction to be executed by model.\n"},
    {PARITY,                0,  ""  ,    "parity"               ,option::Arg::Optional,     "  --parity=<type>              Parity type:\n"
                                                                                            "                               1 - Odd parity.\n"
                                                                                            "                               2 - Even parity.\n"
                                                                                            "                               else - No parity (default).\n"},
    {CMD_FILE,              0,  ""  ,    "cmd-file"             ,option::Arg::Optional,     "  --cmd-file=<file>            Execute file with simulator shell commands.\n"},
    {SHELL,                 0,  ""  ,    "shell"                ,option::Arg::Optional,     "  --shell                      Launch simulator in the interactive shell.\n"},
    {INSTRUCTION_MEM_HEX,   0,  ""  ,    "instruction-mem"      ,option::Arg::Optional,     "  --instruction-mem=<hex-file> Program (assembled) to be loaded to instruction memory.\n"},
    {CONST_ROM_HEX,         0,  ""  ,    "const-rom"            ,option::Arg::Optional,     "  --const-rom=<hex-file>       Content of Constant ROM to be loaded.\n"},
    {DATA_RAM_IN_HEX,       0,  ""  ,    "data-ram-in"          ,option::Arg::Optional,     "  --data-ram-in=<hex-file>     Content of Data RAM IN to be loaded.\n"},
    {DATA_RAM_OUT_HEX,      0,  ""  ,    "data-ram-out"         ,option::Arg::Optional,     "  --data-ram-out=<hex-file>    Path where content of Data RAM out will be dumped.\n"},
    {GRV_HEX,               0,  ""  ,    "grv-hex"              ,option::Arg::Optional,     "  --grv-hex=<hex-file>         Data for GRV instruction (HEX file without address - no '@' in the file). \n"},
    {MAX_INSTR_CNT,         0,  ""  ,    "max-instr-cnt"        ,option::Arg::Optional,     "  --max-instr-cnt=<n>          Limit for number of instructions executed by the simulator. When reached, simulator exits (default = 10^8). Decimal value. \n"},
    {DUMP_CONTEXT,          0,  ""  ,    "dump-context"         ,option::Arg::Optional,     "  --dump-context=<file>        Dump context (state of CPU - GPR registers, Memory content, Hash unit context, RAR stack) after execution to file. \n"},
    {LOAD_CONTEXT,          0,  ""  ,    "load-context"         ,option::Arg::Optional,     "  --load-context=<file>        Load context (state of CPU - GPR registers, Memory content, Hash unit context, RAR stack) before execution from file. \n"},

    {0,0,0,0,0,0}
};

#define EXEC_WITH_ERR_HANDLER(code)                                                                 \
    try {                                                                                           \
            code                                                                                    \
        } catch(std::runtime_error &err) {                                                          \
            std::cout << err.what() << std::endl;                                                   \
            program_exit(1);                                                                        \
        }                                                                                           \

spect::CpuSimulator *simulator;

void program_exit(int ret_code)
{
    delete simulator;
    exit(ret_code);
}


int main(int argc, char** argv)
{
    argc-=(argc>0); argv+=(argc>0);
    option::Stats  stats(usage, argc, argv);
    option::Option options[stats.options_max], buffer[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (parse.error()) {
        option::printUsage(std::cout, usage);
        return 1;
    }

    bool has_unknown = false;
    for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next()) {
        std::cout << "Unknown option: " << opt->name << "\n";
        has_unknown = true;
    }

    if (has_unknown) {
        option::printUsage(std::cout, usage);
        return 1;
    }

    if (options[HELP] || argc == 0) {
        option::printUsage(std::cout, usage);
        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Initialize CPU simulator
    ///////////////////////////////////////////////////////////////////////////////////////////////
    simulator = new spect::CpuSimulator();

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Configure parity type
    ///////////////////////////////////////////////////////////////////////////////////////////////
    spect::ParityType parity_type = spect::ParityType::NONE;
    if (options[PARITY]) {
        if (*options[PARITY].arg == '1')
            parity_type = spect::ParityType::ODD;
        else if (*options[PARITY].arg == '2')
            parity_type = spect::ParityType::EVEN;
    }
    simulator->model_->SetParityType(parity_type);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Compile the program if '.s' file passed on Command line and Preload memories
    ///////////////////////////////////////////////////////////////////////////////////////////////
    uint32_t *m_mem = simulator->model_->GetMemoryPtr();

    if (options[PROGRAM] && options[FIRST_ADDR]) {
        std::stringstream ss;
        ss << std::hex << options[FIRST_ADDR].arg;
        ss >> simulator->compiler_->first_addr_ ;
    } else {
        simulator->compiler_->Warning(
            "'--first-address' undefined, and program loaded via '--program'."
            "First instruction will be placed at start of Instruction memory.");
    }

    if (options[INSTRUCTION_MEM_HEX]) {
        if (options[PROGRAM]) {
            simulator->compiler_->Error(
                "Instruction model invoked with both '--program' and '--instruction-mem'."
                "Use only one source of program (either HEX or .s file)");
        }
        EXEC_WITH_ERR_HANDLER({
            std::string path = std::string(options[INSTRUCTION_MEM_HEX].arg);
            spect::HexHandler::LoadHexFile(path, m_mem, SPECT_INSTR_MEM_BASE);
        })
    }

    if (options[PROGRAM]) {
        std::stringstream ss;
        ss << options[PROGRAM].arg;
        EXEC_WITH_ERR_HANDLER({
            std::string line = std::string(80, '*') + std::string("\n");
            simulator->compiler_->print_fnc(line.c_str());
            simulator->compiler_->print_fnc("Launching SPECT compiler...\n");
            simulator->compiler_->print_fnc(line.c_str());
            simulator->compiler_->Compile(ss.str());
            simulator->compiler_->CompileFinish();
            uint32_t *p_start = m_mem + (simulator->compiler_->program_->first_addr_ >> 2);
            simulator->compiler_->program_->Assemble(p_start, parity_type);
        })
    }

    if (options[CONST_ROM_HEX]) {
        EXEC_WITH_ERR_HANDLER({
            std::string path = std::string(options[CONST_ROM_HEX].arg);
            spect::HexHandler::LoadHexFile(path, m_mem, SPECT_CONST_ROM_BASE);
        })
    }

    if (options[DATA_RAM_IN_HEX]) {
        EXEC_WITH_ERR_HANDLER({
            std::string path = std::string(options[DATA_RAM_IN_HEX].arg);
            spect::HexHandler::LoadHexFile(path, m_mem, SPECT_DATA_RAM_IN_BASE);
        })
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Configure start address
    ///////////////////////////////////////////////////////////////////////////////////////////////
    uint32_t start_pc = SPECT_INSTR_MEM_BASE;
    if (options[PROGRAM]) {
        if (simulator->compiler_->symbols_->IsDefined(START_SYMBOL))
            start_pc = simulator->compiler_->symbols_->GetSymbol(START_SYMBOL)->val_;
        if (options[START_PC])
            simulator->compiler_->Warning(
                "'_start' symbol defined in .s file and '--start-pc' switch is used."
                " Using '--start-pc' as address of first executed instruction.");
    }
    if (options[START_PC]) {
        std::stringstream ss;
        ss << std::hex << options[START_PC].arg;
        ss >> start_pc;
    }
    simulator->model_->SetStartPc(start_pc);

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Configure maximal number of executed instructions
    ///////////////////////////////////////////////////////////////////////////////////////////////
    simulator->model_->max_instr_cnt_ = 10E8;
    if (options[MAX_INSTR_CNT]) {
        std::stringstream ss;
        ss << options[MAX_INSTR_CNT].arg;
        ss >> simulator->model_->max_instr_cnt_;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Feed the GRV data to CPU model
    ///////////////////////////////////////////////////////////////////////////////////////////////
    if (options[GRV_HEX]) {
        std::vector<uint32_t> mem;
        EXEC_WITH_ERR_HANDLER({
            spect::HexHandler::LoadHexFile(std::string(options[GRV_HEX].arg), mem);
        })

        for (const auto &wrd : mem)
            simulator->model_->GrvQueuePush(wrd);
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Run the simulator:
    //      --shell argument - Keep in shell
    //      --cmd-file       - Load commands from cmd-file
    // If --shell is not specified, then run batch and exit.
    ///////////////////////////////////////////////////////////////////////////////////////////////

    bool batch_mode = true;

    if (options[SHELL])
        batch_mode = false;

    if (options[CMD_FILE])
        simulator->cmd_file_ = std::string(options[CMD_FILE].arg);

    if (options[LOAD_CONTEXT])
        simulator->model_context_ = std::string(options[LOAD_CONTEXT].arg);

    EXEC_WITH_ERR_HANDLER({
        simulator->Start(batch_mode);
    })

    if (options[DATA_RAM_OUT_HEX]) {
        spect::HexHandler::DumpHexFile(std::string(options[DATA_RAM_OUT_HEX].arg),
            spect::HexFileType::ISS_WORD, simulator->model_->GetMemoryPtr(), SPECT_DATA_RAM_OUT_BASE,
            SPECT_DATA_RAM_OUT_SIZE);
    }

    if (options[DUMP_CONTEXT]) {
        simulator->model_->DumpContext(std::string(options[DUMP_CONTEXT].arg));
    }

    return 0;
}

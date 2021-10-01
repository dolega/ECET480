#include "llvm/Bitcode/BitcodeReader.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorOr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

static cl::opt<std::string> FileName(cl::Positional,
                                     cl::desc("Bitcode file"), 
                                     cl::Required);

int main(int argc, char** argv) 
{
    cl::ParseCommandLineOptions(argc, argv, 
                                "LLVM hello world\n");
    LLVMContext context;

    ErrorOr<std::unique_ptr<MemoryBuffer>> mb =
        MemoryBuffer::getFile(FileName);
    if (std::error_code ec = mb.getError())
    {
        errs() << ec.message();
        return -1;
    }
    Expected<std::unique_ptr<Module>> m = 
        parseBitcodeFile(mb->get()->getMemBufferRef(), context);

    for (Module::const_iterator i = m.get()->getFunctionList().begin(), 
         e = m.get()->getFunctionList().end(); i != e; ++i) 
    {
        if (!i->isDeclaration())
        {
            outs() << "Function Name: " << i->getName() << "\n";
	    outs() << "Number of lines: " << i->getInstructionCount() << "\n\n";
        }
    }

    return 0;
}


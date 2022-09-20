#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Pass.h"
#include "llvm/Analysis/CFG.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/LegacyPassManager.h"

#include <stdio.h>
#include <map>
#include <fstream>
#include <iostream>
using namespace llvm;

namespace {
	struct CFGPass : public FunctionPass {
		static char ID;
		std::error_code error;
		std::string str;
		//StringMap<int> basicblockMap;
		std::map<BasicBlock*, int> basicBlockMap;
		int bbCount;  //Block的编号
        //为pass注册
		CFGPass() : FunctionPass(ID){
			bbCount = 0;
		}

        //主要是看懂这个runOnFunction函数！
		bool runOnFunction(Function &F) override { //遍历IR，每次遇到一个Function就用此函数处理
			raw_string_ostream rso(str);                //没用的变量
			StringRef name(F.getName().str() + ".dot"); //name: function名字.dot
			
			enum sys::fs::OpenFlags F_None;
			raw_fd_ostream file(name, error, F_None);  //写入名字为name的文件中
			//std::ofstream os;
			//os.open(name.str() + ".dot");
			//if (!os.is_open()){
			//	errs() << "Could not open the " << name << "file\n";
			//	return false;
			//}
			file << "digraph \"CFG for'" + F.getName() + "\' function\" {\n";
			for (Function::iterator B_iter = F.begin(); B_iter != F.end(); ++B_iter){  //遍历function内的没一个basicblock
				BasicBlock* curBB = &*B_iter;
				std::string name = curBB->getName().str();  //得到当前BB的名字，不过这个没用，因为只有同一个module和funtion下的BB的名字才不一样
                                                            //所以map不是<string,int> 而是<BB*,int>
				int fromCountNum;
				int toCountNum;
				if (basicBlockMap.find(curBB) != basicBlockMap.end())  //为每一个basicblock命名，basicBlockMap为<BB，BB编号>的map
				{
					fromCountNum = basicBlockMap[curBB];
				}
				else
				{
					fromCountNum = bbCount;
					basicBlockMap[curBB] = bbCount++;
				}
                //fromCountNum为当前BB的编号
				file << "\tBB" << fromCountNum << " [shape=record, label=\"{";
				file << "BB" << fromCountNum << ":\\l\\l";
                //输出当前块的所有指令
				for (BasicBlock::iterator I_iter = curBB->begin(); I_iter != curBB->end(); ++I_iter) {
					//printInstruction(&*I_iter, os);
					file << *I_iter << "\\l\n";
				}
				file << "}\"];\n";
                //遍历当前块的所有后继块，并且为后继块附上唯一的编号
                //toCountNum为后继BB的编号
				for (BasicBlock *SuccBB : successors(curBB)){
					if (basicBlockMap.find(SuccBB) != basicBlockMap.end())
					{
						toCountNum = basicBlockMap[SuccBB];
					}
					else
					{
						toCountNum = bbCount;
						basicBlockMap[SuccBB] = bbCount++;
					}
                    //输入当前块的编号->后继块的编号
					file << "\tBB" << fromCountNum<< "-> BB"
						<< toCountNum << ";\n";
				}
			}
			file << "}\n";
			file.close();
			return false;
		}
		//void printInstruction(Instruction *inst, std::ofstream os) {

	//}
	};
}
char CFGPass::ID = 0;
// Register for opt
static RegisterPass<CFGPass> X("CFG", "CFG Pass Analyse");

// Register for clang
static RegisterStandardPasses Y(PassManagerBuilder::EP_EarlyAsPossible,
  [](const PassManagerBuilder &Builder, legacy::PassManagerBase &PM) {
    PM.add(new CFGPass());
  });


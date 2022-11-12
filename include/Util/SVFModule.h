//===- SVFModule.h -- SVFModule* class-----------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

/*
 * SVFModule.h
 *
 *  Created on: Aug 4, 2017
 *      Author: Xiaokang Fan
 */

#ifndef SVFMODULE_H_
#define SVFMODULE_H_

#include "Util/BasicTypes.h"
#include "Util/ExtAPI.h"
#include "Util/NodeIDAllocator.h"
#include "Util/ThreadAPI.h"

namespace SVF
{

class SVFModule
{
public:
    typedef std::vector<const SVFFunction*> FunctionSetType;
    typedef std::vector<const Function*> LLVMFunctionSetType;
    typedef std::vector<SVFGlobalValue*> GlobalSetType;
    typedef std::vector<SVFGlobalValue*> AliasSetType;
    typedef Map<const Function*,const SVFFunction*> LLVMFun2SVFFunMap;
    typedef Map<const BasicBlock*,const SVFBasicBlock*> LLVMBB2SVFBBMap;
    typedef Map<const Instruction*,const SVFInstruction*> LLVMInst2SVFInstMap;
    typedef Map<const GlobalValue*,const SVFGlobalValue*> LLVMGlobal2SVFGlobalMap;

    /// Iterators type def
    typedef FunctionSetType::iterator iterator;
    typedef FunctionSetType::const_iterator const_iterator;
    typedef LLVMFunctionSetType::iterator llvm_iterator;
    typedef LLVMFunctionSetType::const_iterator llvm_const_iterator;
    typedef GlobalSetType::iterator global_iterator;
    typedef GlobalSetType::const_iterator const_global_iterator;
    typedef AliasSetType::iterator alias_iterator;
    typedef AliasSetType::const_iterator const_alias_iterator;

private:
    static std::string pagReadFromTxt;
    std::string moduleIdentifier;
    FunctionSetType FunctionSet;  ///< The Functions in the module
    LLVMFunctionSetType LLVMFunctionSet;  ///< The Functions in the module
    GlobalSetType GlobalSet;      ///< The Global Variables in the module
    AliasSetType AliasSet;        ///< The Aliases in the module
    LLVMFun2SVFFunMap LLVMFunc2SVFFunc; ///< Map an LLVM Function to an SVF Function
    LLVMBB2SVFBBMap LLVMBB2SVFBB;
    LLVMInst2SVFInstMap LLVMInst2SVFInst;
    Set<const Value*> argsOfUncalledFunction;
    Set<const SVFInstruction*> returnInsts;
    Set<const Value*> nullPtrSyms;
    Set<const Value*> blackholeSyms;
    Set<const Value*> ptrsInUncalledFunctions;
    Map<const SVFBasicBlock*, const u32_t> bbSuccessorNumMap;
    Map<const PointerType*, const Type*> ptrElementTypeMap;
    // Map<const SVFBasicBlock*, const Map<const SVFBasicBlock*, const u32_t>> bbSuccessorPosMap;
    Map<const SVFBasicBlock*, const Map<const SVFBasicBlock*, const u32_t>> bbPredecessorPosMap;

public:
    /// Constructors
    SVFModule(std::string moduleName = "") : moduleIdentifier(moduleName)
    {
    }

    ~SVFModule()
    {
        for (const SVFFunction* f : FunctionSet)
            delete f;
        for (const SVFGlobalValue* g : GlobalSet)
            delete g;
        for (const SVFGlobalValue* a : AliasSet)
            delete a;
        NodeIDAllocator::unset();
        ThreadAPI::destroy();
        ExtAPI::destory();
    }

    static inline void setPagFromTXT(std::string txt)
    {
        pagReadFromTxt = txt;
    }

    static inline std::string pagFileName()
    {
        return pagReadFromTxt;
    }

    static inline bool pagReadFromTXT()
    {
        if(pagReadFromTxt.empty())
            return false;
        else
            return true;
    }

    void buildSymbolTableInfo();

    ///@{
    inline void addFunctionSet(SVFFunction* svfFunc)
    {
        FunctionSet.push_back(svfFunc);
        LLVMFunctionSet.push_back(svfFunc->getLLVMFun());
        LLVMFunc2SVFFunc[svfFunc->getLLVMFun()] = svfFunc;
    }
    inline void addBasicBlockMap(SVFBasicBlock* svfBB)
    {
        LLVMBB2SVFBB[svfBB->getLLVMBasicBlock()] = svfBB;
    }
    inline void addInstructionMap(SVFInstruction* svfInst)
    {
        LLVMInst2SVFInst[svfInst->getLLVMInstruction()] = svfInst;
    }
    inline void addGlobalSet(SVFGlobalValue* glob)
    {
        GlobalSet.push_back(glob);
    }
    inline void addAliasSet(SVFGlobalValue* alias)
    {
        AliasSet.push_back(alias);
    }
    ///@}

    inline const SVFFunction* getSVFFunction(const Function* fun) const
    {
        LLVMFun2SVFFunMap::const_iterator it = LLVMFunc2SVFFunc.find(fun);
        assert(it!=LLVMFunc2SVFFunc.end() && "SVF Function not found!");
        return it->second;
    }

    inline const SVFBasicBlock* getSVFBasicBlock(const BasicBlock* bb) const
    {
        LLVMBB2SVFBBMap::const_iterator it = LLVMBB2SVFBB.find(bb);
        assert(it!=LLVMBB2SVFBB.end() && "SVF BasicBlock not found!");
        return it->second;
    }

    inline const SVFInstruction* getSVFInstruction(const Instruction* inst) const
    {
        LLVMInst2SVFInstMap::const_iterator it = LLVMInst2SVFInst.find(inst);
        assert(it!=LLVMInst2SVFInst.end() && "SVF Instruction not found!");
        return it->second;
    }

    /// Iterators
    ///@{
    llvm_iterator llvmFunBegin()
    {
        return LLVMFunctionSet.begin();
    }
    llvm_const_iterator llvmFunBegin() const
    {
        return LLVMFunctionSet.begin();
    }
    llvm_iterator llvmFunEnd()
    {
        return LLVMFunctionSet.end();
    }
    llvm_const_iterator llvmFunEnd() const
    {
        return LLVMFunctionSet.end();
    }

    iterator begin()
    {
        return FunctionSet.begin();
    }
    const_iterator begin() const
    {
        return FunctionSet.begin();
    }
    iterator end()
    {
        return FunctionSet.end();
    }
    const_iterator end() const
    {
        return FunctionSet.end();
    }

    global_iterator global_begin()
    {
        return GlobalSet.begin();
    }
    const_global_iterator global_begin() const
    {
        return GlobalSet.begin();
    }
    global_iterator global_end()
    {
        return GlobalSet.end();
    }
    const_global_iterator global_end() const
    {
        return GlobalSet.end();
    }

    alias_iterator alias_begin()
    {
        return AliasSet.begin();
    }
    const_alias_iterator alias_begin() const
    {
        return AliasSet.begin();
    }
    alias_iterator alias_end()
    {
        return AliasSet.end();
    }
    const_alias_iterator alias_end() const
    {
        return AliasSet.end();
    }
    ///@}

    const std::string& getModuleIdentifier() const
    {
        if (pagReadFromTxt.empty())
        {
            assert(moduleIdentifier.empty()==false && "No LLVM module found! Are you reading from a file other than LLVM-IR?");
            return moduleIdentifier;
        }
        else
        {
            return pagReadFromTxt;
        }
    }

    inline const FunctionSetType& getFunctionSet() const
    {
        return FunctionSet;
    }

    inline const Set<const Value*>& getNullPtrSyms() const
    {
        return nullPtrSyms;
    }

    inline const Set<const Value*>& getBlackholeSyms() const
    {
        return blackholeSyms;
    }

    inline const Set<const Value*>& getArgsOfUncalledFunction() const
    {
        return argsOfUncalledFunction;
    }

    inline const Set<const SVFInstruction*>& getReturns() const
    {
        return returnInsts;
    }

    inline const Set<const Value*>& getPtrsInUncalledFunctions() const
    {
        return ptrsInUncalledFunctions;
    }

    inline const Map<const SVFBasicBlock*, const u32_t>& getBBSuccessorNumMap()
    {
        return bbSuccessorNumMap;
    }

    // inline const Map<const SVFBasicBlock*, const Map<const SVFBasicBlock*, const u32_t>>& getBBSuccessorPosMap()
    // {
    //     return bbSuccessorPosMap;
    // }

    inline const Map<const SVFBasicBlock*, const Map<const SVFBasicBlock*, const u32_t>>& getBBPredecessorPosMap()
    {
        return bbPredecessorPosMap;
    }

    inline const Map<const PointerType*, const Type*>& getPtrElementTypeMap()
    {
        return ptrElementTypeMap;
    }

    inline void addBBSuccessorNum(const SVFBasicBlock* bb, const u32_t num)
    {
        bbSuccessorNumMap.insert({bb,num});
    }

    // inline void addBBSuccessorPos(const SVFBasicBlock* bb, const SVFBasicBlock* succ,const u32_t pos)
    // {
    //     Map<const SVFBasicBlock*, const Map<const SVFBasicBlock*, const u32_t>>::iterator bbSuccessorPosMapIter = bbSuccessorPosMap.find(bb);
    //     if(bbSuccessorPosMapIter != bbSuccessorPosMap.end())
    //     {
    //         Map<const SVFBasicBlock*, const u32_t> foundValue = bbSuccessorPosMapIter->second;
    //         foundValue.insert({succ,pos});
    //         bbSuccessorPosMap.insert({bb,foundValue});
    //     }
    //     else
    //     {
    //         Map<const SVFBasicBlock*, const u32_t> valueMap;
    //         valueMap.insert({succ,pos});
    //         bbSuccessorPosMap.insert({bb,valueMap});
    //     }
    // }

    inline void addBBPredecessorPos(const SVFBasicBlock* bb, const SVFBasicBlock* Pred,const u32_t pos)
    {
        Map<const SVFBasicBlock*, const Map<const SVFBasicBlock*, const u32_t>>::iterator bbPredecessorPosMapIter = bbPredecessorPosMap.find(bb);
        if(bbPredecessorPosMapIter != bbPredecessorPosMap.end())
        {
            Map<const SVFBasicBlock*, const u32_t> foundValue = bbPredecessorPosMapIter->second;
            foundValue.insert({Pred,pos});
            bbPredecessorPosMap.insert({bb,foundValue});
        }
        else
        {
            Map<const SVFBasicBlock*, const u32_t> valueMap;
            valueMap.insert({Pred,pos});
            bbPredecessorPosMap.insert({bb,valueMap});
        }
    }

    inline void addptrElementType(const PointerType* ptrType, const Type* type)
    {
        ptrElementTypeMap.insert({ptrType, type});
    }

    inline void addPtrInUncalledFunction (const Value * value)
    {
        ptrsInUncalledFunctions.insert(value);
    }

    inline void addNullPtrSyms(const Value *val)
    {
        nullPtrSyms.insert(val);
    }

    inline void addBlackholeSyms(const Value *val)
    {
        blackholeSyms.insert(val);
    }

    inline void addArgsOfUncalledFunction(const Value *val)
    {
        argsOfUncalledFunction.insert(val);
    }

    inline void addReturn(const SVFInstruction* inst)
    {
        returnInsts.insert(inst);
    }

};

} // End namespace SVF

#endif /* SVFMODULE_H_ */

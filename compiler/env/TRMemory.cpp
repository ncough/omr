/*******************************************************************************
 * Copyright (c) 2000, 2016 IBM Corp. and others
 *
 * This program and the accompanying materials are made available under
 * the terms of the Eclipse Public License 2.0 which accompanies this
 * distribution and is available at http://eclipse.org/legal/epl-2.0
 * or the Apache License, Version 2.0 which accompanies this distribution
 * and is available at https://www.apache.org/licenses/LICENSE-2.0.
 *
 * This Source Code may also be made available under the following Secondary
 * Licenses when the conditions for such availability set forth in the
 * Eclipse Public License, v. 2.0 are satisfied: GNU General Public License,
 * version 2 with the GNU Classpath Exception [1] and GNU General Public
 * License, version 2 with the OpenJDK Assembly Exception [2].
 *
 * [1] https://www.gnu.org/software/classpath/license.html
 * [2] http://openjdk.java.net/legal/assembly-exception.html
 *
 * SPDX-License-Identifier: EPL-2.0 OR Apache-2.0
 *******************************************************************************/

#include <algorithm>                    // for std::min, etc
#include <new>                          // for bad_alloc
#include <stddef.h>                     // for size_t, NULL
#include <stdint.h>                     // for int32_t, uint32_t, uint8_t, etc
#include <stdio.h>                      // for fprintf, stderr
#include "codegen/FrontEnd.hpp"         // for TR_FrontEnd, feGetEnv
#include "compile/Compilation.hpp"      // for Compilation, etc
#include "control/Options.hpp"
#include "control/Options_inlines.hpp"  // for TR::Options, etc
#include "env/IO.hpp"
#include "env/TRMemory.hpp"
#include "il/DataTypes.hpp"             // for pointer_cast
#include "infra/Assert.hpp"             // for TR_ASSERT, TR_ASSUMES

#ifdef J9_PROJECT_SPECIFIC
#include "env/VMJ9.h"
#endif

namespace TR { class Monitor; }

extern size_t mem_round(size_t);

const char * objectName[] =
   {
   "Array",
   "Instruction",
   "ListElement",
   "Node",
   "BitVector",
   "Register",
   "MemoryReference",
   "CFGNode",
   "Symbol",
   "SymbolReference",
   "SymRefUnion",
   "BackingStore",
   "StorageReference",
   "PseudoRegister",
   "Link",
   "Machine",
   "CFGEdge",
   "TreeTop",
   "GCStackMap",
   "IlGenerator",
   "RegisterDependencyConditions",
   "Snippet",
   "Relocation",
   "VirtualGuard",
   "RegisterDependencyGroup",
   "RegisterDependency",
   "FrontEnd",
   "VMField",
   "VMFieldsInfo",
   "Method",
   "Pair",
   "HashTab",
   "HashTable",
   "HashTabEntry",
   "DebugCounter",
   "JSR292",
   "Memory",
   "CS2",
   "UnknownType",

   // Optimizer Types
   "AVLTree",
   "CFGChecker",
   "CFGSimplifier",
   "CatchBlockRemover",
   "CompactLocals",
   "CopyPropagation",
   "DataFlowAnalysis",
   "DominatorVerifier",
   "Dominators",
   "DominatorsChk",
   "EscapeAnalysis",
   "EstimateCodeSize",
   "ExpressionsSimplification",
   "GlobalRegisterAllocator",
   "GlobalRegisterCandidates",
   "HedgeTree",
   "Inliner",
   "InnerPreexistence",
   "IsolatedStoreElimination",
   "LocalLiveVariablesForGC",
   "LocalAnalysis",
   "LocalCSE",
   "LocalDeadStoreElimination",
   "LocalOpts",
   "LocalReordering",
   "LocalLiveRangeReduction",
   "LongRegAllocation",
   "LoopAliasRefiner",
   "LoopTransformer",
   "MonitorElimination",
   "NewInitialization",
   "Optimization",
   "OSR",
   "PartialRedundancy",
   "RedundantAsycCheckRemoval",
   "RegionAnalysis",
   "RightNumberOfNames",
   "Structure",
   "SwitchAnalyzer",
   "UseDefInfo",
   "ValueNumberInfo",
   "ValuePropagation",
   "VirtualGuardTailSplitter",
   "InterProceduralAnalyzer",
   "InductionVariableAnalysis",
   "CoarseningInterProceduralAnalyzer",
   "ShrinkWrapping",

   "AheadOfTimeCompile",
   "HWProfile",
   "TR_HWProfileCallSiteElem",
   "TR_HWProfileCallSiteList",

   // ARM types
   "ARMRelocation",
   "ARMMemoryArgument",
   "ARMOperand",
   "ARMFloatConstant",
   "ARMRegisterDependencyGroup",
   "ARMRegisterDependencyConditions",
   "ARMConstant",
   "ARMConstantDataSnippet",

   "BlockCloner",
   "BlockFrequencyInfo",
   "ByteCodeIterator",
   "CallSiteInfo",
   "CatchBlockExtension",
   "CatchBlockProfileInfo",
   "CFG",
   "CHTable",
   "ClassLookahead",
   "CodeGenerator",
   "Compilation",
   "ExceptionTableEntry",
   "ExceptionTableEntryIterator",
   "ExtendedBlockSuccessorIterator",
   "ExtraInfoForNew",
   "GlobalRegister",
   "GCRegisterMap",
   "GCStackAtlas",
   "GRABlockInfo",
   "IdAddrNodes",

   // IA32 codegen
   "BetterSpillPlacement",
   "ClobberingInstruction",
   "OutlinedCode",
   "IA32ProcessorInfo",
   "X86AllocPrefetchGeometry",
   "X86TOCHashTable",

   "IGBase",
   "IGNode",
   "InternalFunctionsBase",
   "InternalPointerMap",
   "InternalPointerPair",
   "LinkHead",
   "Linkage",
   "List",
   "ListAppender",
   "ListIterator",
   "LiveReference",
   "LiveRegisterInfo",
   "LiveRegisters",
   "NodeMappings",
   "Optimizer",

   "Options",
   "OptionSet",
   "OrderedExceptionHandlerIterator",
   "PCMapEntry",
   "PersistentCHTable",
   "PersistentInfo",
   "PersistentMethodInfo",
   "PersistentJittedBodyInfo",
   "PersistentProfileInfo",

   // PPC objects
   "PPCFloatConstant",
   "PPCConstant",
   "PPCConstantDataSnippet",
   "PPCCounter",
   "PPCLoadLabelItem",
   "PPCRelocation",
   "PPCMemoryArgument",

   "SnippetHashtable",
   "RandomGenerator",
   "Recompilation",
   "RegisterCandidates",
   "RematerializationInfo",

   // S390 types
   "S390Relocation",
   "S390ProcessorInfo",
   "S390MemoryArgument",
   "S390EncodingRelocation",
   "BranchPreloadCallData",

   "ScratchRegisterManager",
   "SimpleRegex",
   "SimpleRegex::Component",
   "SimpleRegex::Regex",
   "SimpleRegex::Simple",
   "SingleTimer",
   "SymbolReferenceTable",
   "TableOfConstants",
   "Timer",
   "TwoListIterator",
   "ValueProfileInfo",
   "ValueProfileInfoManager",
   "MethodValueProfileInfo",
   "MethodBranchProfileInfo",
   "BranchProfileInfoManager",
   "VirtualGuardSiteInfo",
   "WCode",
   "CallGraph",
   "CPOController",
   "RegisterIterator",
   "TRStats",
   "TR_OptimizationPlan",
   "TR_ClassHolder",
   "SmartBuffer",
   "TR_ListingInfo",
   "methodInfoType",

   "IProfiler",
   "TR_IPBytecodeHashTableEntry",
   "TR_IPBCDataFourBytes",
   "TR_IPBCDataAllocation",
   "TR_IPBCDataEightWords",
   "TR_IPBCDataPointer",
   "TR_IPBCDataCallGraph",
   "TR_ICallingContext",

   "TR_IPMethodTable",
   "TR_IPCCNode",
   "IPHashedCallSite",
   "Assumptions",
   "PatchSites",

   "TR_ZHWProfiler",
   "TR_PPCHWProfiler",
   "TR_ZGuardedStorage",
   "TR_PPCLMGuardedStorage",

   "CatchTable",

   "DecimalPrivatization",
   "DecimalReduction",

   "TR_PreviousNodeConversion",
   "TR::RelocationDebugInfo",
   "TR::AOTClassInfo",
   "TR_SharedCache",

   "TR::RegisterPair",
   "TR::Instruction",

   "TR_ArtifactManager",
   "CompilationInfo",
   "CompilationInfoPerThreadBase",
   "TR_DebuggingCounters",
   "TR_Pattern",
   "TR_UseNodeInfo",
   "PPSEntry",
   "GPUHelpers",
   "TR_ColdVariableOutliner",

   "CodeMetaData",
   "CodeMetaDataAVL",

   "RegionAllocation",

   "Debug",

   };


// **************************************************************************
//
// deprecated uses of the global deprecatedTRMemory (not thread safe)
//

extern TR_PersistentMemory * trPersistentMemory;

// **************************************************************************
//
//
//

TR_Memory::TR_Memory(TR_PersistentMemory &persistentMemory, TR::Region & heapMemoryRegion) :
   TR_MemoryBase(persistentMemory),
   _trPersistentMemory(&persistentMemory),
   _comp(NULL),
   _heapMemoryRegion(heapMemoryRegion),
   _stackMemoryRegion(TR::ref(heapMemoryRegion))
   {
   TR_ASSERT(sizeof(objectName)/sizeof(char*) == NumObjectTypes, "Mismatch between NumObjectTypes and object names");
   }

// **************************************************************************
//
// Heap allocation
//

void *
TR_Memory::allocateHeapMemory(size_t requestedSize, ObjectType ot)
   {
   void *alloc = _heapMemoryRegion.allocate(requestedSize);
   TR::AllocatedMemoryMeter::update_allocated(requestedSize, heapAlloc);
   return alloc;
   }
void *
TR_Memory::allocateStackMemory(size_t requestedSize, ObjectType ot)
   {
   void * alloc = _stackMemoryRegion.get().allocate(requestedSize);
   TR::AllocatedMemoryMeter::update_allocated(requestedSize, stackAlloc);
   return alloc;
   }

TR::Region &
TR_Memory::currentStackRegion()
   {
   return _stackMemoryRegion.get();
   }

TR::Region &
TR_Memory::registerStackRegion(TR::Region &stackMemoryRegion)
   {
   TR::Region &currentRegion = _stackMemoryRegion;
   _stackMemoryRegion = TR::ref(stackMemoryRegion);
   return currentRegion;
   }

void
TR_Memory::unregisterStackRegion(TR::Region &current, TR::Region &previous) throw()
   {
   TR_ASSERT(current == _stackMemoryRegion.get(), "Mismatched stack regions");
   _stackMemoryRegion = TR::ref(previous);
   }

void *
TR_Memory::allocateMemory(size_t size, TR_AllocationKind kind, ObjectType ot)
   {
   switch(kind)
      {
      case heapAlloc:       return allocateHeapMemory(size, ot);
      case stackAlloc:      return allocateStackMemory(size, ot);
      case persistentAlloc:
         {
         void * alloc = _trPersistentMemory->allocatePersistentMemory(size, ot);
         if (!alloc) throw std::bad_alloc();
         return alloc;
         }
      default:              return allocateHeapMemory(size, ot);
      }
   }

void
TR_Memory::freeMemory(void *p, TR_AllocationKind kind, ObjectType ot)
   {
   switch(kind)
      {
      case persistentAlloc:
         _trPersistentMemory->freePersistentMemory(p);
         break;
      default:
         (void) 0;
         break;
      }
   }

// **************************************************************************
//
// allocation helpers
//

namespace TR
   {
   GlobalSingletonAllocator* GlobalSingletonAllocator::_instance = NULL;

   void GlobalSingletonAllocator::createInstance()
      {
      TR_ASSERT(::trPersistentMemory != NULL, "Attempting to use GlobalAllocator too early. It cannot be used until TR_PersistentMemory has been initialized.");
      static CS2PersistentAllocator persistentAllocator(::trPersistentMemory);
      static GlobalBaseAllocator globalBaseAllocator(persistentAllocator);
      static GlobalSingletonAllocator globalSingletonAllocator(globalBaseAllocator);
      }

   AllocatedMemoryMeter::Metric AllocatedMemoryMeter::_currentMemUsage = 0;
   // profile everything until read options
   uint8_t AllocatedMemoryMeter::_enabled = heapAlloc | stackAlloc | persistentAlloc;
   }

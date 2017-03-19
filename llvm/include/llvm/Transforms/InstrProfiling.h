//===- Transforms/InstrProfiling.h - Instrumentation passes -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file provides the interface for LLVM's PGO Instrumentation lowering
/// pass.
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTRPROFILING_H
#define LLVM_TRANSFORMS_INSTRPROFILING_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/PassManager.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Transforms/Instrumentation.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace llvm {

class TargetLibraryInfo;

/// Instrumentation based profiling lowering pass. This pass lowers
/// the profile instrumented code generated by FE or the IR based
/// instrumentation pass.
class InstrProfiling : public PassInfoMixin<InstrProfiling> {
public:
  InstrProfiling() = default;
  InstrProfiling(const InstrProfOptions &Options) : Options(Options) {}

  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  bool run(Module &M, const TargetLibraryInfo &TLI);

private:
  InstrProfOptions Options;
  Module *M;
  const TargetLibraryInfo *TLI;
  struct PerFunctionProfileData {
    uint32_t NumValueSites[IPVK_Last + 1];
    GlobalVariable *RegionCounters = nullptr;
    GlobalVariable *DataVar = nullptr;

    PerFunctionProfileData() {
      memset(NumValueSites, 0, sizeof(uint32_t) * (IPVK_Last + 1));
    }
  };
  DenseMap<GlobalVariable *, PerFunctionProfileData> ProfileDataMap;
  std::vector<GlobalValue *> UsedVars;
  std::vector<GlobalVariable *> ReferencedNames;
  GlobalVariable *NamesVar;
  size_t NamesSize;

  // The start value of precise value profile range for memory intrinsic sizes.
  const int64_t DefaultMemOPSizeRangeStart = 0;
  int64_t MemOPSizeRangeStart;
  // The end value of precise value profile range for memory intrinsic sizes.
  const int64_t DefaultMemOPSizeRangeLast = 8;
  int64_t MemOPSizeRangeLast;
  int64_t MemOPSizeLargeVal;

  bool isMachO() const;

  /// Get the section name for the counter variables.
  StringRef getCountersSection() const;

  /// Get the section name for the name variables.
  StringRef getNameSection() const;

  /// Get the section name for the profile data variables.
  StringRef getDataSection() const;

  /// Get the section name for the coverage mapping data.
  StringRef getCoverageSection() const;

  /// Count the number of instrumented value sites for the function.
  void computeNumValueSiteCounts(InstrProfValueProfileInst *Ins);

  /// Replace instrprof_value_profile with a call to runtime library.
  void lowerValueProfileInst(InstrProfValueProfileInst *Ins);

  /// Replace instrprof_increment with an increment of the appropriate value.
  void lowerIncrement(InstrProfIncrementInst *Inc);

  /// Force emitting of name vars for unused functions.
  void lowerCoverageData(GlobalVariable *CoverageNamesVar);

  /// Get the region counters for an increment, creating them if necessary.
  ///
  /// If the counter array doesn't yet exist, the profile data variables
  /// referring to them will also be created.
  GlobalVariable *getOrCreateRegionCounters(InstrProfIncrementInst *Inc);

  /// Emit the section with compressed function names.
  void emitNameData();

  /// Emit value nodes section for value profiling.
  void emitVNodes();

  /// Emit runtime registration functions for each profile data variable.
  void emitRegistration();

  /// Emit the necessary plumbing to pull in the runtime initialization.
  void emitRuntimeHook();

  /// Add uses of our data variables and runtime hook.
  void emitUses();

  /// Create a static initializer for our data, on platforms that need it,
  /// and for any profile output file that was specified.
  void emitInitialization();

  /// Helper funtion that parsing the MemOPSize value profile options
  void getMemOPSizeOptions();
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_INSTRPROFILING_H
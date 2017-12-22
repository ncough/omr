/*******************************************************************************
 * Copyright (c) 2017, 2017 IBM Corp. and others
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

#include <gtest/gtest.h>
#include "Jit.hpp"
#include "JitTest.hpp"
#include "default_compiler.hpp"

uint8_t byteValues[] = {
   0x00,
   0x01,
   0x80,
   0x81,
   0xF1,
   0xF0,
   0x0F,
   0xFF,
};
uint8_t byteMasks[] = {
   0x00,
   0xF0,
   0x0F,
   0xFF,
   0xAA,
   0x61,
   0x60,
   0x09,
};

uint16_t shortValues[] = {
   0x0000,
   0x0001,
   0x8000,
   0x8181,
   0xF001,
   0xFF00,
   0x00FF,
   0xFFFF,
};
uint16_t shortMasks[] = {
   0x0000,
   0xFF00,
   0x00FF,
   0xFFFF,
   0xAAAA,
   0x6161,
   0x5050,
   0x0909,
};

uint32_t intValues[] = {
   0x00000000,
   0x00000001,
   0x80000000,
   0x81818181,
   0xF0000001,
   0xFFFF0000,
   0x0000FFFF,
   0xFFFFFFFF,
};
uint32_t intMasks[] = {
   0x00000000,
   0xFFFF0000,
   0x0000FFFF,
   0xFFFFFFFF,
   0xAAAAAAAA,
   0x61616161,
   0x50505050,
   0x09090909,
};

uint64_t longValues[] = {
   0x0000000000000000,
   0x0000000000000001,
   0x8000000000000000,
   0x8181818181818181,
   0xF000000000000001,
   0xFFFFFFFF00000000,
   0x00000000FFFFFFFF,
   0xFFFFFFFFFFFFFFFF,
};
uint64_t longMasks[] = {
   0x0000000000000000,
   0xFFFFFFFF00000000,
   0x00000000FFFFFFFF,
   0xFFFFFFFFFFFFFFFF,
   0xAAAAAAAAAAAAAAAA,
   0x6161616161616161,
   0x5050505050505050,
   0x0909090909090909,
};


template <typename VarType>
class MaskExtractTest : public ::testing::TestWithParam<std::tuple<VarType, VarType, VarType (*) (VarType, VarType)>>
   {
   public:

   static void SetUpTestCase()
      {
      const char *options = "-Xjit:acceptHugeMethods,enableBasicBlockHoisting,omitFramePointer,useILValidator,paranoidoptcheck,disableAsyncCompilation";
      auto initSuccess = initializeJitWithOptions(const_cast<char*>(options));

      ASSERT_TRUE(initSuccess) << "Failed to initialize JIT.";
      }

   static void TearDownTestCase()
      {
      shutdownJit();
      }
   };

template <typename VarType>
struct MaskExtractStruct
   {
   VarType value;
   VarType mask;
   VarType (*oracle)(VarType, VarType);
   };

template <typename VarType>
MaskExtractStruct<VarType> to_struct(const std::tuple<VarType, VarType, VarType (*) (VarType, VarType)> &param)
   {
   MaskExtractStruct<VarType> s;

   s.value = std::get<0>(param);
   s.mask = std::get<1>(param);

   return s;
   }

template <typename VarType>
static VarType maskExtractOracle(VarType value, VarType mask)
   {
   size_t i = 1;
   VarType result = 0;
   while (mask)
      {
      VarType bit = mask & ~(mask - 1);
      if (value & bit)
         result |= i;
      work &= ~bit;
      i = i << 1;
      }
   return result;
   }

class lMaskExtract : public MaskExtractTest<uint64_t> {};

TEST_P(lMaskExtractTest, SimpleTest)
   {
   auto param = to_struct(GetParam());

   char inputTrees[512] = {0};
   std::snprintf(inputTrees, sizeof(inputTrees),
      "(method return=Int64 args=[Int64, Int64]  "
      " (block                                   "
      "  (lreturn                                "
      "  (lmaskextract                           "
      "   (lload parm=0)                         "
      "   (lload parm=1)                         "
      "  ))))                                    "
      );

   auto trees = parseString(inputTrees);

   ASSERT_NOTNULL(trees);

   Tril::DefaultCompiler compiler {trees};

   ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly\nInput trees: " << inputTrees;

   auto entry_point = compiler.getEntryPoint<uint64_t (*)(uint64_t, uint64_t)>();

   ASSERT_EQ(param.oracle(param.value, param.mask), entry_point(param.value, param.mask));
   }

class iMaskExtract : public MaskExtractTest<uint32_t> {};

TEST_P(iMaskExtractTest, SimpleTest)
   {
   auto param = to_struct(GetParam());

   char inputTrees[512] = {0};
   std::snprintf(inputTrees, sizeof(inputTrees),
      "(method return=Int32 args=[Int32, Int32]  "
      " (block                                   "
      "  (ireturn                                "
      "  (imaskextract                           "
      "   (iload parm=0)                         "
      "   (iload parm=1)                         "
      "  ))))                                    "
      );

   auto trees = parseString(inputTrees);

   ASSERT_NOTNULL(trees);

   Tril::DefaultCompiler compiler {trees};

   ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly\nInput trees: " << inputTrees;

   auto entry_point = compiler.getEntryPoint<uint32_t (*)(uint32_t, uint32_t)>();

   ASSERT_EQ(param.oracle(param.value, param.mask), entry_point(param.value, param.mask));
   }

class sMaskExtract : public MaskExtractTest<uint16_t> {};

TEST_P(sMaskExtractTest, SimpleTest)
   {
   auto param = to_struct(GetParam());

   char inputTrees[512] = {0};
   std::snprintf(inputTrees, sizeof(inputTrees),
      "(method return=Int32 args=[Int16, Int16]  "
      " (block                                   "
      "  (ireturn                                "
      "  (su2i                                   "
      "  (smaskextract                           "
      "   (sload parm=0)                         "
      "   (sload parm=1)                         "
      "  )))))                                   "
      );

   auto trees = parseString(inputTrees);

   ASSERT_NOTNULL(trees);

   Tril::DefaultCompiler compiler {trees};

   ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly\nInput trees: " << inputTrees;

   auto entry_point = compiler.getEntryPoint<uint32_t (*)(uint16_t, uint16_t)>();

   ASSERT_EQ(param.oracle(param.value, param.mask), entry_point(param.value, param.mask));
   }

class bMaskExtract : public MaskExtractTest<uint8_t> {};

TEST_P(bMaskExtractTest, SimpleTest)
   {
   auto param = to_struct(GetParam());

   char inputTrees[512] = {0};
   std::snprintf(inputTrees, sizeof(inputTrees),
      "(method return=Int32 args=[Int8, Int8] "
      " (block                                "
      "  (ireturn                             "
      "  (bu2i                                "
      "  (bmaskextract                        "
      "   (bload parm=0)                      "
      "   (bload parm=1)                      "
      "  )))))                                "
      );

   auto trees = parseString(inputTrees);

   ASSERT_NOTNULL(trees);

   Tril::DefaultCompiler compiler {trees};

   ASSERT_EQ(0, compiler.compile()) << "Compilation failed unexpectedly\nInput trees: " << inputTrees;

   auto entry_point = compiler.getEntryPoint<uint32_t (*)(uint8_t, uint8_t)>();

   ASSERT_EQ(param.oracle(param.value, param.mask), entry_point(param.value, param.mask));
   }

#ifdef TR_TARGET_X86
INSTANTIATE_TEST_CASE_P(lMaskExtract, lMaskExtractTest,
              ::testing::Combine(
                 ::testing::ValuesIn(longValues),
                 ::testing::ValuesIn(longMasks),
                 ::testing::Values(static_cast<uint64_t (*) (uint64_t, uint64_t)> (maskExtractOracle))));

INSTANTIATE_TEST_CASE_P(iMaskExtract, iMaskExtractTest,
              ::testing::Combine(
                 ::testing::ValuesIn(intValues),
                 ::testing::ValuesIn(intMasks),
                 ::testing::Values(static_cast<uint32_t (*) (uint32_t, uint32_t)> (maskExtractOracle))));

INSTANTIATE_TEST_CASE_P(sMaskExtract, sMaskExtractTest,
              ::testing::Combine(
                 ::testing::ValuesIn(shortValues),
                 ::testing::ValuesIn(shortMasks),
                 ::testing::Values(static_cast<uint16_t (*) (uint16_t, uint16_t)> (maskExtractOracle))));

INSTANTIATE_TEST_CASE_P(bMaskExtract, bMaskExtractTest,
              ::testing::Combine(
                 ::testing::ValuesIn(byteValues),
                 ::testing::ValuesIn(byteMasks),
                 ::testing::Values(static_cast<uint8_t (*) (uint8_t, uint8_t)> (maskExtractOracle))));
#endif

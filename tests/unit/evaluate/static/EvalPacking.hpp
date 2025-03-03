/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file EvalPacking.hpp 
 *  @brief Tests the Evaluate function for EvalPacking.hpp. 
 */

// Empirical tools
#include "emp/base/vector.hpp"
// MABE
#include "evaluate/static/EvalPacking.hpp"

TEST_CASE("EvalPacking_Evaluate", "[Evaluate/static]"){
  {
    // Setup a MABE object (with a population!) as prerequirements for EvalPacking
    mabe::MABE control = mabe::MABE(0, NULL);
    control.AddPopulation("fake pop"); 
    mabe::EvalPacking packing(control);

    emp::BitVector bits("00");
    emp::BitVector bits1("000011110000");
    emp::BitVector bits2("11110000111100");
    emp::BitVector bits3("00000111011100");
    emp::BitVector bits4("111111111");
    emp::BitVector bits5(""); 
    emp::BitVector bits6("00011"); 
    
    // signature: Evaluate(bits, min_padding, package_size);

    // Package of ones is larger than what was expected 
    CHECK(packing.EvaluateOrg(bits, 2, 3) == 0);
    CHECK(packing.EvaluateOrg(bits4, 0, 20) == 0);
    // Successful package of ones 
    CHECK(packing.EvaluateOrg(bits1, 4, 4) == 1); 
    // Successful package of ones, no padding required if starting at beginning 
    CHECK(packing.EvaluateOrg(bits2, 2, 4) == 2); 
    // Interior padding (of the right length) can be reused
    CHECK(packing.EvaluateOrg(bits3, 1, 3) == 2); 
    CHECK(packing.EvaluateOrg(bits3, 2, 3) == 0);
    // Works even with padding == 0
    CHECK(packing.EvaluateOrg(bits4, 0, 3) == 3);
    // Code runs even with 0-length packages
    CHECK(packing.EvaluateOrg(bits4, 3, 0) == 0);
    // Ensure extra padding is acceptable
    CHECK(packing.EvaluateOrg(bits1, 3, 4) == 1);
    CHECK(packing.EvaluateOrg(bits1, 2, 4) == 1);
    CHECK(packing.EvaluateOrg(bits1, 1, 4) == 1);
    // Successful package of ones, no padding required if package ends at end of bitstring 
    CHECK(packing.EvaluateOrg(bits6, 3, 2) == 1);
  }
}

/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2023.
 *
 *  @file  NK-MultiAllele.cpp
 *  @brief TODO. Currently this is a placeholder so codecov will see the untested source code
 */

// CATCH
#define CATCH_CONFIG_MAIN
#include "catch.hpp"
// MABE
#include "tools/NK-MultiAllele.hpp"


TEST_CASE("NK-MultiAllele_Defaults", "[core]"){ 
  mabe::NKLandscapeMultiAllele landscape;
  CHECK(landscape.GetN() == 0); 
  CHECK(landscape.GetK() == 0); 
  CHECK(landscape.GetAlphabetSize() == 0); 
}

TEST_CASE("NK-MultiAllele_Base", "[core]"){ 
  // Initialize variables
  size_t N = 5;
  size_t K = 1;
  size_t alphabet_size = 3;
  size_t random_seed = 7;
  emp::Random random(random_seed);
  mabe::NKLandscapeMultiAllele landscape(N, K, alphabet_size, random);

  // Test getters
  CHECK(landscape.GetN() == N); 
  CHECK(landscape.GetK() == K); 
  CHECK(landscape.GetAlphabetSize() == alphabet_size); 

  // Check our math
  // With K = 1 and three symbols, we expect 3 * 3 possible states per site
  CHECK(landscape.GetStateCount() == 9);
  // With 9 possible states per site and N = 5, we expect 9 * 5 fitness contributions 
  //    in the table
  CHECK(landscape.GetTotalCount() == 45);
}


TEST_CASE("NK-MultiAllele_Simple", "[core]"){ 
  // Initialize variables
  size_t N = 3;
  size_t K = 1;
  size_t alphabet_size = 3;
  size_t random_seed = 8;
  emp::Random random(random_seed);
  mabe::NKLandscapeMultiAllele landscape(N, K, alphabet_size, random);

  // Test getters
  CHECK(landscape.GetN() == N); 
  CHECK(landscape.GetK() == K); 
  CHECK(landscape.GetAlphabetSize() == alphabet_size); 

  // Check our math
  // With K = 1 and three symbols, we expect 3 * 3 possible states per site
  CHECK(landscape.GetStateCount() == 9);
  // With 9 possible states per site and N = 3, we expect 9 * 3 fitness contributions 
  //    in the table
  CHECK(landscape.GetTotalCount() == 27);

  // Fill out the table 
  // (THIS CURRENTLY SHOULD THROW AN ERROR BECAUSE TABLE IS A VECTOR OF EMPTY VECTORS)
  {
    // Site 0
    landscape.SetState(0, 0, 0.0); // AA
    landscape.SetState(0, 1, 1.0); // AB *
    landscape.SetState(0, 2, 0.0); // AC
    landscape.SetState(0, 3, 0.0); // BA
    landscape.SetState(0, 4, 0.0); // BB
    landscape.SetState(0, 5, 0.0); // BC
    landscape.SetState(0, 6, 0.0); // CA
    landscape.SetState(0, 7, 0.0); // CB
    landscape.SetState(0, 8, 0.0); // CC
    // Site 1
    landscape.SetState(1, 0, 0.0); // AA
    landscape.SetState(1, 1, 0.0); // AB
    landscape.SetState(1, 2, 0.0); // AC
    landscape.SetState(1, 3, 0.0); // BA
    landscape.SetState(1, 4, 0.0); // BB
    landscape.SetState(1, 5, 1.0); // BC *
    landscape.SetState(1, 6, 0.0); // CA
    landscape.SetState(1, 7, 0.0); // CB
    landscape.SetState(1, 8, 0.0); // CC
    // Site 2
    landscape.SetState(2, 0, 0.0); // AA
    landscape.SetState(2, 1, 0.0); // AB
    landscape.SetState(2, 2, 0.0); // AC
    landscape.SetState(2, 3, 0.0); // BA
    landscape.SetState(2, 4, 0.0); // BB
    landscape.SetState(2, 5, 0.0); // BC
    landscape.SetState(2, 6, 1.0); // CA *
    landscape.SetState(2, 7, 0.0); // CB
    landscape.SetState(2, 8, 0.0); // CC
  }
  // Test each value in the table
  {
    // Site 0
    CHECK(landscape.GetState(0, 0) == 0.0); // AA
    CHECK(landscape.GetState(0, 1) == 1.0); // AB *
    CHECK(landscape.GetState(0, 2) == 0.0); // AC
    CHECK(landscape.GetState(0, 3) == 0.0); // BA
    CHECK(landscape.GetState(0, 4) == 0.0); // BB
    CHECK(landscape.GetState(0, 5) == 0.0); // BC
    CHECK(landscape.GetState(0, 6) == 0.0); // CA
    CHECK(landscape.GetState(0, 7) == 0.0); // CB
    CHECK(landscape.GetState(0, 8) == 0.0); // CC
    // Site 1
    CHECK(landscape.GetState(1, 0) ==  0.0); // AA
    CHECK(landscape.GetState(1, 1) ==  0.0); // AB 
    CHECK(landscape.GetState(1, 2) ==  0.0); // AC 
    CHECK(landscape.GetState(1, 3) ==  0.0); // BA
    CHECK(landscape.GetState(1, 4) ==  0.0); // BB
    CHECK(landscape.GetState(1, 5) ==  1.0); // BC *
    CHECK(landscape.GetState(1, 6) ==  0.0); // CA
    CHECK(landscape.GetState(1, 7) ==  0.0); // CB
    CHECK(landscape.GetState(1, 8) ==  0.0); // CC
    // Site 2
    CHECK(landscape.GetState(2, 0) ==  0.0); // AA
    CHECK(landscape.GetState(2, 1) ==  0.0); // AB
    CHECK(landscape.GetState(2, 2) ==  0.0); // AC
    CHECK(landscape.GetState(2, 3) ==  0.0); // BA
    CHECK(landscape.GetState(2, 4) ==  0.0); // BB
    CHECK(landscape.GetState(2, 5) ==  0.0); // BC
    CHECK(landscape.GetState(2, 6) ==  1.0); // CA *
    CHECK(landscape.GetState(2, 7) ==  0.0); // CB
    CHECK(landscape.GetState(2, 8) ==  0.0); // CC
  }
 
  // Check a few specific genomes
  { // ABC - Best genome
    emp::vector<size_t> genome{1, 5, 6};
    CHECK(landscape.GetFitness(genome) == 3.0);
  } 
  { // CBA - Zero genome
    emp::vector<size_t> genome{7, 3, 2};
    CHECK(landscape.GetFitness(genome) == 0.0);
  } 
  { // ABA - Only first site confers fitness 
    emp::vector<size_t> genome{1, 3, 1};
    CHECK(landscape.GetFitness(genome) == 0.0);
  } 

}

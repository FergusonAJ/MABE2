/**
 *  @note This file is part of Empirical, https://github.com/devosoft/Empirical
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2023.
 *
 *  @file  NK-MultiAllele.hpp
 *  @brief This file provides code to build NK landscapes with more than two alleles.
 *
 *  TODO list: 
 *    - Come up with a variable name for the number of alleles
 *      - In automata theory, the alphabet is defined by Sigma
 *      - In that case, the number of symbols is |Sigma|, but that's clunky
 *      - Currently called `alphabet_size`
 *    - Figure out best data type for genomes
 *      - vector<size_t> is an obvious choice
 *      - We could create an "fixed_alphabet_character" that does bounds checking in debug
 *    - Ensure that the lookup table scheme still works
 *      - Any changes needed to the variable?
 *    - Fix/add code where marked with TODO in this file
 */

#ifndef MABE_TOOL_NK_MULTI_ALLELE_H
#define MABE_TOOL_NK_MULTI_ALLELE_H

#include "emp/base/vector.hpp"
#include "emp/bits/BitVector.hpp"
#include "emp/functional/memo_function.hpp"
#include "emp/math/math.hpp"
#include "emp/math/Random.hpp"

namespace mabe {

  /// To see a description of general NK landscapes, please see NK.hpp
  /// Here we extend the bitstring NK landscapes to have more than two possible alleles
  /// This alphabet size is configurable. 
  /// Otherwise, the landscape functions exactly the same
  class NKLandscapeMultiAllele {
  public: 
    using genome_t = emp::vector<size_t>; /** The type of a string of symbols,
                                                subject to change! */

  private:
    size_t N;             ///< The number of symbols in each genome.
    size_t K;             ///< The number of OTHER bits with which each bit is epistatic.
    size_t alphabet_size; ///< The number of possible alleles
    size_t state_count;   ///< The total number of states associated with each bit table.
    size_t total_count;   ///< The total number of states in the entire landscape space.
    emp::vector< emp::vector<double> > landscape;  ///< The actual values in the landscape.

  public:
    NKLandscapeMultiAllele() : N(0), K(0), alphabet_size(0), state_count(0), total_count(0), landscape(){
      ; 
    }
    NKLandscapeMultiAllele(const NKLandscapeMultiAllele &) = default;
    NKLandscapeMultiAllele(NKLandscapeMultiAllele &&) = default;

    /// N is the length of strings in your population, 
    /// K is the number of neighboring sites
    ///   the affect the fitness contribution of each site (i.e. epistasis or ruggedness), 
    /// alphabet_size is the number of possible alleles for any given site
    /// random is the random number generator to use to generate this landscape.
    // TODO: Fix logic for state_count
    // TODO: Fix logic for total_count
    NKLandscapeMultiAllele(size_t _N, size_t _K, size_t _alphabet_size, emp::Random & random)
     : N(_N), K(_K), alphabet_size(_alphabet_size)
     , state_count(emp::IntPow<size_t>(2,K+1))
     , total_count(N * state_count)
     , landscape(N)
    {
      Reset(random);
    }
    ~NKLandscapeMultiAllele() { ; }
    NKLandscapeMultiAllele & operator=(const NKLandscapeMultiAllele &) = delete;
    NKLandscapeMultiAllele & operator=(NKLandscapeMultiAllele &&) = default;

    /// Randomize the landscape without changing the landscape size.
    // TODO: implement by randomizing table
    void Reset(emp::Random & random) {
    }

    /// Configure for new values
    // TODO: Set variables and reset
    void Config(size_t _N, size_t _K, size_t _alphabet_size, emp::Random & random) {
    }

    /// Returns N
    size_t GetN() const { return N; }
    /// Returns K
    size_t GetK() const { return K; }
    /// Returns the number of possible alleles for a given site
    size_t GetAlphabetSize() const { return alphabet_size; }
    /// Get the number of posssible states for a given site
    size_t GetStateCount() const { return state_count; }
    /// Get the total number of states possible in the landscape
    ///   (i.e. the number of different fitness contributions in the table)
    size_t GetTotalCount() const { return total_count; }

    /// Get the fitness contribution of position [n] when it (and its K neighbors) have the 
    ///   value [state]
    // TODO: Implement via table lookup
    double GetFitness(size_t n, size_t state) const {
      return 0;
    }

    /// Get the fitness of a whole string
    /// TODO: Implement via multiple table lookups
    double GetFitness( std::vector<size_t> states ) const {
      return 0;
    }

    /// Get the fitness of a whole string (pass by value so can be modified.)
    // Implement by constructing states for each site and using them for table lookup
    double GetFitness(emp::BitVector genome) const {
      return 0;
    }

    /// Set the fitness value for state at a given site
    // TODO: add bounds checking
    void SetState(size_t site, size_t state, double in_fit) { 
      landscape[site][state] = in_fit; 
    }

    /// Get the fitness value for state at a given site
    // TODO: add bounds checking?
    double GetState(size_t site, size_t state) const { return landscape[site][state]; }

    /// Ranomize exactly num_states states in the table
    // TODO: Make sure this still makes sense
    // TODO: Randomize all states (how is this different than Reset?
    void RandomizeStates(emp::Random & random, size_t num_states=1) {
    }

  };

}

#endif

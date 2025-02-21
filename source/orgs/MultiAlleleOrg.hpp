/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2023.
 *
 *  @file  MultiAlleleOrg.hpp
 *  @brief An organism consisting of a series of symbols.
 *  @note Status: ALPHA
 */

#ifndef MABE_MULTIALLELE_ORGANISM_H
#define MABE_MULTIALLELE_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"
#include "emp/bits/BitVector.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class MultiAlleleOrg : public OrganismTemplate<MultiAlleleOrg> {
  public:
    using genome_t = emp::vector<size_t>; 
    
  protected:
    genome_t genome;

  public:
    MultiAlleleOrg(OrganismManager<MultiAlleleOrg> & _manager)
      : OrganismTemplate<MultiAlleleOrg>(_manager), genome(100) { }
    MultiAlleleOrg(const MultiAlleleOrg &) = default;
    MultiAlleleOrg(MultiAlleleOrg &&) = default;
    MultiAlleleOrg(const genome_t & in, OrganismManager<MultiAlleleOrg> & _manager)
      : OrganismTemplate<MultiAlleleOrg>(_manager), genome(in) { }
    MultiAlleleOrg(size_t N, OrganismManager<MultiAlleleOrg> & _manager)
      : OrganismTemplate<MultiAlleleOrg>(_manager), genome(N) { }
    ~MultiAlleleOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of each bit mutating on reproduction.
      size_t num_alleles = 3;            /// number of possible alleles in our genome(currently called genome)
      std::string output_name = "genome";  ///< Name of trait that should be used to access genome.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites. 
      bool init_random = true;           ///< Should we randomize ancestor?  (false = all zeros)
    };

    /// Use "to_string" to convert.
    emp::String ToString() const override { return emp::to_string(genome); }

    size_t Mutate(emp::Random & random) override {
     const size_t num_muts = SharedData().mut_dist.PickRandom(random);

      if (num_muts == 0) return 0;
      if (num_muts == 1) {
        const size_t pos = random.GetUInt(genome.size());
        size_t allele = random.GetUInt(SharedData().num_alleles);
        while(allele == genome[pos]){
            allele = random.GetUInt(SharedData().num_alleles);
        }
        genome[pos] = allele;
        return 1;
      }

      // Only remaining option is num_muts > 1.
      auto & mut_sites = SharedData().mut_sites;
      mut_sites.Clear();
      for (size_t i = 0; i < num_muts; i++) {
        const size_t pos = random.GetUInt(genome.size());
        if (mut_sites[pos]) { --i; continue; }  // Duplicate position; try again.
        mut_sites.Set(pos);
        size_t allele = random.GetUInt(SharedData().num_alleles);
        while(allele == genome[pos]){
            allele = random.GetUInt(SharedData().num_alleles);
        }
        genome[pos] = allele;
      }
      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      const size_t min = 0;
      const size_t max = SharedData().num_alleles;
      emp::RandomizeVector(genome, random, min, max);  
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random); 
    }

    /// Put the bits in the correct output position.
    void GenerateOutput() override {
      SetTrait<genome_t>(SharedData().output_name, genome);   
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkFuns<size_t>([this](){ return genome.size(); },
                       [this](const size_t & N){ 
                        return genome.resize(N); 
                      },
                       "N", "Number of symbols in organism");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each symbols mutating on reproduction.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain symbols sequence.");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = all zeros)");
      GetManager().LinkVar(SharedData().num_alleles, "num_alleles",
                      "Number of possible alleles in our genome");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the mutation distribution.
      SharedData().mut_dist.Setup(SharedData().mut_prob, genome.size());

      // Setup the default vector to indicate mutation positions.
      SharedData().mut_sites.Resize(genome.size());

      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Bitset output from organism.",
                                  genome_t(0)); 
    }
  };

  MABE_REGISTER_ORG_TYPE(MultiAlleleOrg, "Organism consisting of a series of N symbols.");
}

#endif

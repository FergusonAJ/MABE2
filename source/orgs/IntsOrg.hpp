/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2022.
 *
 *  @file  IntsOrg.hpp
 *  @brief An organism consisting of N integers
 *  @note Status: ALPHA
 */

#ifndef MABE_INTS_ORGANISM_H
#define MABE_INTS_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

namespace mabe {

  class IntsOrg : public OrganismTemplate<IntsOrg> {
  protected:
    // Vector stored as trait on org

  public:
    IntsOrg(OrganismManager<IntsOrg> & _manager)
      : OrganismTemplate<IntsOrg>(_manager){ }
    IntsOrg(const IntsOrg &) = default;
    IntsOrg(IntsOrg &&) = default;
    ~IntsOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of each int mutating on reproduction.
      std::string output_name = "ints";  ///< Name of trait that should be used to access bits.
      emp::Binomial mut_dist;            ///< Distribution of number of mutations to occur.
      emp::BitVector mut_sites;          ///< A pre-allocated vector for mutation sites. 
      bool init_random = true;           ///< Should we randomize ancestor?  (false = all zeros)
      int min_val = 0;
      int max_val = 100;
      size_t vec_size = 10;
    };

    /// Use "to_string" to convert.
    std::string ToString() const override { 
      const emp::vector<int>& ints = GetTrait<emp::vector<int>>(SharedData().output_name);
      return emp::to_string(ints); 
    }

    size_t Mutate(emp::Random & random) override {
      size_t num_muts = 0;
      emp::vector<int>& ints = GetTrait<emp::vector<int>>(SharedData().output_name);
      for(int& val : ints){
        if(random.P(SharedData().mut_prob)){
          if(random.P(0.5)){
            if(val > SharedData().min_val){
              num_muts++;
              val--;
            }
          }
          else{ 
            if(val < SharedData().max_val){
              num_muts++;
              val++;
            }
          }
        }
      }
      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      emp::vector<int>& ints = GetTrait<emp::vector<int>>(SharedData().output_name);
      for(int& val : ints){
        val = random.GetInt(SharedData().min_val, SharedData().max_val);
      }
    }

    void Initialize(emp::Random & random) override {
      emp::vector<int>& ints = GetTrait<emp::vector<int>>(SharedData().output_name);
      ints.resize(SharedData().vec_size, SharedData().min_val);
      if (SharedData().init_random) Randomize(random);
    }

    /// Do nothing. Data already in place.
    void GenerateOutput() override {
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkFuns<size_t>([this](){ return SharedData().vec_size; },
                       [this](const size_t & N){ 
                          SharedData().vec_size = N;
                          return SharedData().vec_size;
                       },
                       "N", "Number of ints in organism");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each int mutating on reproduction.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain int vector.");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = all zeros)");
      GetManager().LinkVar(SharedData().min_val, "min_val",
                      "Minimum value");
      GetManager().LinkVar(SharedData().max_val, "max_val",
                      "Maximum value");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Int vector output from organism.",
                                  emp::vector<int>(0));
    }
    
    /// Load in a genome from a string
    /// Genome format: "[ a b c ]"
    void GenomeFromString(const std::string & new_genome) override {
      emp::vector<int>& ints = GetTrait<emp::vector<int>>(SharedData().output_name);
      emp::vector<std::string> sliced_vec = emp::slice(new_genome, ' ');
      emp_assert(sliced_vec.size() == SharedData().vec_size + 2); // +2 for [ and ]
      ints.resize(SharedData().vec_size);
      for(size_t idx = 0; idx < SharedData().vec_size; ++idx){
        ints[idx] = std::stoi(sliced_vec[idx + 1]); // +1 to skip [
        emp_assert(ints[idx] >= SharedData().min_val);
        emp_assert(ints[idx] <= SharedData().max_val);
      }
    }
  };

  MABE_REGISTER_ORG_TYPE(IntsOrg, "Organism consisting of a series of N integers.");
}

#endif

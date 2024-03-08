/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  EvalNK.hpp
 *  @brief MABE Evaluation module for NK Landscapes
 */

#ifndef MABE_EVAL_NK_H
#define MABE_EVAL_NK_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"
#include "../../tools/NK.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalNK : public Module {
  private:
    size_t N;
    size_t K;    
    NKLandscape landscape;

    std::string bits_trait;
    std::string fitness_trait;
    std::string loci_fitness_trait = "loci_fitnesses";
    
    bool calculate_loci_fitnesses = false;

  public:
    EvalNK(mabe::MABE & control,
           const std::string & name="EvalNK",
           const std::string & desc="Module to evaluate bitstrings on an NK Fitness Lanscape",
           size_t _N=100, size_t _K=3, const std::string & _btrait="bits", const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , N(_N), K(_K)
      , bits_trait(_btrait)
      , fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
    }
    ~EvalNK() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalNK & mod, Collection list) { return mod.Evaluate(list); },
                             "Use NK landscape to evaluate all orgs in an OrgList.");
      info.AddMemberFunction("RESET",
                             [](EvalNK & mod) { mod.landscape.Config(mod.N, mod.K, mod.control.GetRandom()); return 0; },
                             "Regenerate the NK landscape with current N and K.");
    }

    void SetupConfig() override {
      LinkVar(N, "N", "Number of bits required in output");
      LinkVar(K, "K", "Number of bits used in each gene");
      LinkVar(bits_trait, "bits_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
      LinkVar(loci_fitness_trait, "loci_fitness_trait", "Which trait should we store NK fitness of each loci?");
      LinkVar(calculate_loci_fitnesses, "calculate_loci_fitnesses", "Should we calculate and store the fitness of each loci?");
    }

    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::BitVector>(bits_trait);
      AddOwnedTrait<double>(fitness_trait, "NK fitness value", 0.0);
      AddOwnedTrait<emp::vector<double>>(loci_fitness_trait, "NK fitness value for each loci", { });

      // Setup the fitness landscape.
      landscape.Config(N, K, control.GetRandom());  // Setup the fitness landscape.
    }

    double Evaluate(const Collection & orgs) {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        org.GenerateOutput();
        const auto & bits = org.GetTrait<emp::BitVector>(bits_trait);
        if (bits.size() != N) {
          emp::notify::Error("Org returns ", bits.size(), " bits, but ",
                             N, " bits needed for NK landscape.",
                             "\nOrg: ", org.ToString());
        }
        double fitness = 0; // Calculated in either branch of this if
        if(calculate_loci_fitnesses){
          auto & loci_fitnesses = org.GetTrait<emp::vector<double>>(loci_fitness_trait);
          if(loci_fitnesses.size() != N) loci_fitnesses.resize(N, 0);
          for(size_t locus_index = 0; locus_index < N; ++locus_index){
            const double locus_fitness = landscape.GetFitnessOfLocus(bits, locus_index);
            loci_fitnesses[locus_index] = locus_fitness;
            fitness += locus_fitness;
          }
        } 
        else{
          fitness = landscape.GetFitness(bits);
        }
        org.SetTrait<double>(fitness_trait, fitness);
        if (fitness > max_fitness || !max_org) {
          max_fitness = fitness;
          max_org = &org;
        }
      }

      return max_fitness;
    }

    // If a population is provided to Evaluate, first convert it to a Collection.
    double Evaluate(Population & pop) { return Evaluate( Collection(pop) ); }

    // If a string is provided to Evaluate, convert it to a Collection.
    double Evaluate(const std::string & in) { return Evaluate( control.ToCollection(in) ); }
  };

  MABE_REGISTER_MODULE(EvalNK, "Evaluate bitstrings on an NK fitness lanscape.");
}

#endif

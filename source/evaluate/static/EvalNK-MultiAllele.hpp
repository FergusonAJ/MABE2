/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2023.
 *
 *  @file  EvalNK-MultiAllele.hpp
 *  @brief MABE Evaluation module for NK-MultiAllele Landscapes.
 */

#ifndef MABE_EVAL_NK_MULTI_ALLELE_H
#define MABE_EVAL_NK_MULTI_ALLELE_H

#include "../../core/EvalModule.hpp"
#include "../../tools/NK-MultiAllele.hpp"

#include "emp/datastructs/reference_vector.hpp"

namespace mabe {

  class EvalNKMultiAllele : public EvalModule<EvalNKMultiAllele> {
  public:
    using genome_t = emp::vector<size_t>; 

  protected:
    genome_t genome;

  private:
    //MABE_REQUIRED_TRAIT(genome, genome_t, "genome-sequence to evaluate.");  
    RequiredTrait<genome_t> genome_trait{this, "genome", "genome-sequence to evaluate."}; 
    OwnedTrait<double> fitness_trait{this, "fitness", "NK-MultiAllele fitness value"};

    size_t N = 100;
    size_t K = 2;    
    size_t num_alleles = 3; 
    NKLandscapeMultiAllele landscape;

  public:
    EvalNKMultiAllele(mabe::MABE & control,
           const std::string & name="EvalNKMultiAllele",
           const std::string & desc="Evaluate bitstrings on an NK-MutiAllele Fitness Landscape")
      : EvalModule(control, name, desc) { }
    ~EvalNKMultiAllele() { }

    void SetupConfig() override {
      LinkVar(N, "N", "Total number of symbols required in sequence");
      LinkVar(K, "K", "Number of symbols used in each gene");
      LinkVar(num_alleles, "num_alleles", "The number of possible alleles");
    }

    void SetupModule() override {
      // Setup the fitness landscape.
      landscape.Config(N, K, num_alleles, control.GetRandom());  // Setup the fitness landscape.
    }

    double Evaluate(const Collection & orgs) override {
     // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        org.GenerateOutput();
        const auto & genome = genome_trait(org);     
        if (genome.size() != N) {   
           emp::notify::Error("Org returns ", genome.size(), " alleles, but ",
                             N, " alleles needed for NK-MultiAllele landscape.",
                             "\nOrg: ", org.ToString());
        }
        const double fitness = landscape.GetFitness(genome);
        fitness_trait(org) = fitness;

        if (fitness > max_fitness || !max_org) {
           max_fitness = fitness;
           max_org = &org;
        }
      }

      return max_fitness;
    }

    /// Re-randomize all of the entries.
    double Reset() override {
      landscape.Config(N, K, num_alleles, control.GetRandom());
      return 0.0;
    }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                             [](EvalNKMultiAllele & mod, Collection list) { return mod.Evaluate(list); },
                             "Evaluate all orgs in the OrgList.");
      info.AddMemberFunction("RESET",
                             [](EvalNKMultiAllele & mod) { return mod.Reset(); }, 
                             "Regenerate the landscape with current config values.");
      info.AddMemberFunction("GET_FITNESS_STATE",
                             [](EvalNKMultiAllele & mod, size_t n , size_t state ) {
                                return mod.landscape.GetFitnessState(n, state);
                              },
                             "This calculates the fitness.");
    }

  };


  MABE_REGISTER_MODULE(EvalNKMultiAllele, "Evaluate bitstrings on an NK-MultiAllele fitness landscape.\nFor more info about NK models, see: https://en.wikipedia.org/wiki/NK_model");
}

#endif

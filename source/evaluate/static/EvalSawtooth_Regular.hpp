/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  EvalSawtooth_Regular.hpp
 *  @brief MABE Evaluation module for regular (repeating) sawtooth functions
 */

#ifndef MABE_EVAL_SAWTOOTH_REGULAR_H
#define MABE_EVAL_SAWTOOTH_REGULAR_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalSawtooth_Regular : public Module {
  private:
    size_t N = 1;
    size_t valley_width = 4;
    double step_decrease = 1;
    int offset = 0;

    std::string ints_trait = "ints";
    std::string fitness_trait = "fitness";

  public:
    EvalSawtooth_Regular(mabe::MABE & control,
           const std::string & name="EvalSawtooth_Regular",
           const std::string & desc="Module to evaluate integer vectors on a repeating sawtooth")
      : Module(control, name, desc){
      SetEvaluateMod(true);
    }
    ~EvalSawtooth_Regular() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                              [](EvalSawtooth_Regular & mod, Collection list) { 
                                return mod.Evaluate(list); 
                             },
                             "Use sawtooth to evaluate all orgs in an OrgList.");
    }
    
    void SetupConfig() override {
      LinkVar(N, "N", "Number of ints required in output");
      LinkVar(ints_trait, "ints_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
      LinkVar(valley_width, "valley_width", "Number of steps from one peak to the next");
      LinkVar(step_decrease, "step_decrease", 
          "Fitness penalty for each step into the valley");
      LinkVar(offset, "offset", "X offset of the sawtooth (changes which values are peaks)");
    }

    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::vector<int>>(ints_trait);
      AddOwnedTrait<double>(fitness_trait, "Sawtooth fitness value", 0.0);
    }

    double GetSawtoothValue(int val){
      return val - (2 * step_decrease * ((val - offset) % valley_width));
    }

    double Evaluate(const Collection & orgs) {
      // Loop through the population and evaluate each organism.
      double max_fitness = 0.0;
      emp::Ptr<Organism> max_org = nullptr;
      mabe::Collection alive_orgs( orgs.GetAlive() );
      for (Organism & org : alive_orgs) {
        org.GenerateOutput();
        const auto & ints = org.GetTrait<emp::vector<int>>(ints_trait);
        if (ints.size() != N) {
          emp::notify::Error("Org returns ", ints.size(), " ints, but ",
                             N, " ints needed for EvalSawtooth_Regular.",
                             "\nOrg: ", org.ToString());
        }
        double fitness = 0;
        for(int val : ints){
          fitness += GetSawtoothValue(val);
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

  MABE_REGISTER_MODULE(EvalSawtooth_Regular, "Evaluate integer vectors on a sawtooth lanscape.");
}

#endif

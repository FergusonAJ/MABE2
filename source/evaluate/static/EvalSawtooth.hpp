/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2021.
 *
 *  @file  EvalSawtooth.hpp
 *  @brief MABE Evaluation module for NK Landscapes
 */

#ifndef MABE_EVAL_SAWTOOTH_H
#define MABE_EVAL_SAWTOOTH_H

#include "../../core/MABE.hpp"
#include "../../core/Module.hpp"

namespace mabe {

  class EvalSawtooth : public Module {
  private:
    size_t N = 10;
    emp::vector<int> sawtooth_vals;
    int sawtooth_min_peak = 8;           // First peak in the sawtooth diagnostic
    size_t max_val = 100;

    std::string ints_trait = "ints";
    std::string fitness_trait = "fitness";

  public:
    EvalSawtooth(mabe::MABE & control,
           const std::string & name="EvalSawtooth",
           const std::string & desc="Module to evaluate integer vectors on a sawtooth")
      : Module(control, name, desc){
      SetEvaluateMod(true);
    }
    ~EvalSawtooth() { }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("EVAL",
                              [](EvalSawtooth & mod, Collection list) { 
                                return mod.Evaluate(list); 
                             },
                             "Use sawtooth to evaluate all orgs in an OrgList.");
    }
    
    void SetupSawtooth(){
      sawtooth_vals.clear();
      int next_val = sawtooth_min_peak;
      int old_val = -1;
      int step = 1;
      int dist = 0;
      std::cout << "Sawtooth peaks: [";
      for(int i = 0; i <= max_val; i++){
        dist++;
        if(i == next_val){
          old_val = next_val;
          next_val = old_val + step;
          step++;
          dist = 0;
        } 
        if(i != 0) std::cout << ", ";
        if(old_val == -1){
          std::cout << old_val;
          sawtooth_vals.push_back(old_val);
        }
        else{
          std::cout << old_val - dist;
          sawtooth_vals.push_back(old_val - dist);
        }
      }
      std::cout << "]" << std::endl;
    }

    void SetupConfig() override {
      LinkVar(N, "N", "Number of ints required in output");
      LinkVar(ints_trait, "ints_trait", "Which trait stores the bit sequence to evaluate?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
      LinkVar(sawtooth_min_peak, "sawtooth_min_peak", 
          "The first peak in the sawtooth");
      LinkVar(max_val, "max_val", 
          "The maximum value a single integer can take");
    }

    void SetupModule() override {
      // Setup the traits.
      AddRequiredTrait<emp::vector<int>>(ints_trait);
      AddOwnedTrait<double>(fitness_trait, "Sawtooth fitness value", 0.0);
      SetupSawtooth();
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
                             N, " ints needed for EvalSawtooth.",
                             "\nOrg: ", org.ToString());
        }
        double fitness = 0;
        for(int val : ints){
          fitness += sawtooth_vals[val];
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

  MABE_REGISTER_MODULE(EvalSawtooth, "Evaluate integer vectors on a sawtooth lanscape.");
}

#endif

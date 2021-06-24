/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020-2021.
 *
 *  @file  Combinator.hpp
 *  @brief Module to combine two traits of an organism into a single trait (via several possible operations) 
 */

#ifndef MABE_COMBINATOR_H
#define MABE_COMBINATOR_H

#include "../core/Module.hpp"

namespace mabe {

  /// Add elite selection with the current population.
  class Combinator : public Module {
  private:
    Collection org_collection;
    //std::vector<std::string> trait_vec;
    std::string trait_1 = "trait_1"; 
    std::string trait_2 = "trait_2"; 
    std::string output_trait = "output_trait"; 
    std::string op; 

  public:
    Combinator(mabe::MABE & control,
           const std::string & name="Combinator",
           const std::string & desc="Combine two organism traits into one using various operators",
           const std::string & _trait_1 = "trait_1", 
           const std::string & _trait_2 = "trait_2", 
           const std::string & _output_trait = "output_trait", 
           const std::string & _op = "+")
      : Module(control, name, desc) 
      , org_collection(control.GetPopulation(0))
      , trait_1(_trait_1) 
      , trait_2(_trait_2) 
      , output_trait(_output_trait) 
      , op(_op) 
    {
      SetEvaluateMod(true);
    }
    

    void SetupConfig() override {
      LinkCollection(org_collection, "collection", "Which population(s) should we operate on?");
      LinkVar(trait_1, "trait_1", "Name of the first trait");
      LinkVar(trait_2, "trait_2", "Name of the second trait");
      LinkVar(output_trait, "output_trait", "Name of the output trait");
      LinkVar(op, "operator", "Which operation to perform");
    }

    void SetupModule() override {
      AddRequiredTrait<double>(trait_1);
      //AddRequiredTrait<double>(trait_2);
      AddOwnedTrait<double>(output_trait, "Combined trait", 0.0);
    }

    void OnUpdate(size_t /* update */) override {
      mabe::Collection alive_collect( org_collection.GetAlive() );
      if(op == "+"){
        for (Organism & org : alive_collect) {        
          org.SetVar<double>(output_trait, org.GetVar<double>(trait_1) + org.GetVar<double>(trait_2));
        }
      }
    }
  };

  MABE_REGISTER_MODULE(Combinator, "Combine two organism traits into one using various operators");
}

#endif

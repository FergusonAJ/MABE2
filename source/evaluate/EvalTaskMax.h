/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  EvalTaskMax.h
 *  @brief This is JUST A TEST (should be deleted later) to demonstrate task eval via triggers
 *
 * This was created to see if an organism can request to be evaluated by external modules
 *    This is set up like an Avida task (e.g., nand, not), but since it takes a while for even NOT to 
 *    evolve, this just looks for an output > 1 and sets fitness to that output.  
 */

#ifndef MABE_EVAL_TASK_MAX_H
#define MABE_EVAL_TASK_MAX_H

#include "../core/MABE.h"
#include "../core/Module.h"


namespace mabe {

  class EvalTaskMax : public Module {
  private:
    std::string outputs_trait;
    std::string fitness_trait;

  public:
    EvalTaskMax(mabe::MABE & control,
                  const std::string & name="EvalTaskMax",
                  const std::string & desc="{TEST} Evaluate organism on MAX via triggers",
                  const std::string & _otrait="outputs",
                  const std::string & _ftrait="fitness")
      : Module(control, name, desc)
      , outputs_trait(_otrait)
      , fitness_trait(_ftrait)
    {
      SetEvaluateMod(true);
    }
    ~EvalTaskMax() { }

    void SetupConfig() override {
      LinkVar(outputs_trait, "outputs_trait", "Which trait containts the organism's outputs?");
      LinkVar(fitness_trait, "fitness_trait", "Which trait should we store NK fitness in?");
    }

    void SetupModule() override {
      AddRequiredTrait<std::unordered_map<int,double>>(outputs_trait);
      AddOwnedTrait<double>(fitness_trait, "All-ones fitness value", 1.0);
    }

    // Check outputs of org, if first output > 1 then set fitness to that value
    void OnManualEval(Organism& org) override {
        const std::unordered_map<int,double> & outputs = 
            org.GetVar<std::unordered_map<int,double>>(outputs_trait);
        if(outputs.size() > 0){
          const double output = outputs.begin()->second;
          org.SetVar<double>(fitness_trait, output > 1 ? output : 1);
        }
        else
          org.SetVar<double>(fitness_trait, 1);
    }
  };

  MABE_REGISTER_MODULE(EvalTaskMax, "Test module for evaluating tasks via triggers");
}

#endif
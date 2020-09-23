/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2020.
 *
 *  @file  DumpPopulation.h
 *  @brief Prints all organisms in a population to std::cout 
 */

#ifndef MABE_SCHEMA_DUMP_POPULATION_H
#define MABE_SCHEMA_DUMP_POPULATION_H

#include "../core/Module.h"

namespace mabe {

  class DumpPopulation : public Module {
  private:
    int pop_id = 1;       ///< Which population are we moving from?

  public:
    DumpPopulation(mabe::MABE & control,
           const std::string & name="DumpPopulation",
           const std::string & desc="Prints all organisms in a population to std::cout",
           int _pop_id = 0)
      : Module(control, name, desc), pop_id(_pop_id)
    {
    }

    void SetupConfig() override {
      LinkPop(pop_id, "pop", "Population to print.");
      std::function<int(size_t)> write_func =
        [this](size_t update) {
          WriteToFile(update);
          return 0;
        };
      GetScope().AddFunction("write", write_func,
        "Writes the assigned population to the console.");

    }
    
    void WriteToFile(size_t update){
      Population & pop = control.GetPopulation(pop_id);
      std::cout << "Dumping organisms from pop (update: " << update << "): " << std::endl;
      for(size_t org_idx = 0; org_idx < pop.GetNumOrgs(); ++org_idx){
        std::cout << "\t" << pop[org_idx].ToString() << std::endl;
      }  
    }

    void OnUpdate(size_t update) override {
    }
  };

  MABE_REGISTER_MODULE(DumpPopulation, "Print all organisms in a populaiton to std::cout.");
}

#endif

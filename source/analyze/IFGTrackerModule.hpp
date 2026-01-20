/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2022.
 *
 *  @file  SystematicsModule.hpp
 *  @brief MABE systematic tracking module.
 */

#ifndef MABE_IFG_TRACKER_MODULE_H
#define MABE_IFG_TRACKER_MODULE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "../core/TraitSet.hpp"
  
namespace mabe {

class IFGTracker : public Module {
private:
  //std::unordered_map<std::tuple<int,int,int>, size_t> map;
  std::unordered_map<int, std::unordered_map<int, std::unordered_map<int, size_t>>> map;
  std::string genotype_trait = "genotype";

  // Convert 
  size_t LocalHash(const emp::vector<int>& v){
  }

public:
    IFGTracker(mabe::MABE & control,
               const std::string & name="IFGTracker",
               const std::string & desc="Module to track the population's exploration of an iterated fitness graph")
      : Module(control, name, desc)
    {
      SetAnalyzeMod(true);    ///< Mark this module as an analyze module.
    }
    ~IFGTracker() { }

    void SetupConfig() override {
      LinkVar(genotype_trait, "genotype_trait", "Which trait stores the genotypes to track?");
    }

    void SetupModule() override {
      // Setup the traits.
      // Setup the traits.
      AddRequiredTrait<emp::vector<int>>(genotype_trait);
    }
    
    size_t UpdateMap(const Collection & orgs, size_t update){
      size_t discovery_count = 0;
      for(size_t org_idx = 0; org_idx < orgs.size(); ++org_idx){
        const Organism& org = orgs[org_idx];
        const auto & genotype = org.GetTrait<emp::vector<int>>(genotype_trait);
        const int source_node = genotype[0];
        const int dest_node = genotype[1];
        const int num_steps = genotype[2];
        //auto tuple = std::make_tuple(genotype[0], genotype[1], genotype[2]);
        if(!emp::Has(map[source_node][dest_node], num_steps)){
          map[source_node][dest_node][num_steps] = update;
          discovery_count++;
        }
      }
      return discovery_count;
    }

    void Print(std::ostream & str){
      str << "source_node,dest_node,num_steps,update_discovered\n";
      for(auto it_source = map.begin(); it_source != map.end(); it_source++){
        const auto & source_map = it_source->second;
        for(auto it_dest = source_map.begin(); it_dest != source_map.end(); it_dest++){
          const auto & dest_map = it_dest->second;
          for(auto it_steps = dest_map.begin(); it_steps != dest_map.end(); it_steps++){
            str << it_source->first << "," 
                      << it_dest->first << "," 
                      << it_steps->first << "," 
                      << it_steps->second << "\n";
          }
        }
      }
    }
      
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("UPDATE",
          [](IFGTracker & mod, Collection collection, size_t update) { 
            return mod.UpdateMap(collection, update);
          },
          "Track the given organisms");
      info.AddMemberFunction("PRINT",
          [](IFGTracker & mod) { 
            mod.Print(std::cout);
            return 0;
          },
          "Print all data");
      info.AddMemberFunction("WRITE_TO_FILE",
          [](IFGTracker & mod, const std::string& filename) { 
            std::ofstream fp;
            fp.open(filename);
            mod.Print(fp);
            fp.close();
            return 0;
          },
          "Save all data to file");
    }
};

    MABE_REGISTER_MODULE(IFGTracker, "Module to track organisms progress through an iterated fitness graph (IFG)");
}
#endif

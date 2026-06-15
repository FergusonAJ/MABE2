/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2026
 *
 *  @file  PatchPlacement.h
 *  @brief Manages multiple patches of organisms, with each being an externally created MABE population. Allows populations to be linked with potential migration between them.
 *
 */

#ifndef MABE_PATCH_PLACEMENT_H
#define MABE_PATCH_PLACEMENT_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"

namespace mabe {

  /// Creates a network of patches, each a population, which organisms can sometimes migrate between
  class PatchPlacement : public Module {
  private:
    using pop_ptr_t = emp::Ptr<mabe::Population>;

    Collection collection;    ///< Collection of populations to manage
    emp::vector<size_t> tracked_pop_ids; 
    std::unordered_map<size_t, std::unordered_map<size_t, double>> connections;
    std::unordered_map<std::string, emp::Ptr<Collection>> collection_map;
    std::unordered_map<std::string, std::unordered_map<std::string, double>> collection_connections;


  public:
    PatchPlacement(mabe::MABE & control,
        const std::string & name="PatchPlacement",
        const std::string & desc="Treats external populations as connected patches")
      : Module(control, name, desc), collection(), tracked_pop_ids(), connections()
    {
      SetPlacementMod(true);
    }
    ~PatchPlacement() { }

    /// Set up variables for configuration file
    void SetupConfig() override {
    }

    void SetupModule() override { ; }
    
    /// Add a patch without a name. 
    void AddPatch(Population& pop){
      size_t pop_id = pop.GetID();
      emp_assert(std::find(tracked_pop_ids.begin(), tracked_pop_ids.end(), pop_id) == tracked_pop_ids.end(), "Can't add the same population more than once");
      tracked_pop_ids.push_back(pop_id);
      collection.Insert(pop);
      connections[pop_id] = {};
    }

    void AddCollection(Collection& collection, const std::string& name){
      emp_assert(!emp::Has(collection_map, name), "Cannot reuse collection names");
      collection_map[name] = emp::Ptr<Collection>(&collection);
      collection_connections[name] = {};
    }

    void AddConnection(Population& pop_a, Population& pop_b, double prob){
      size_t id_a = pop_a.GetID();
      emp_assert(std::find(tracked_pop_ids.begin(), tracked_pop_ids.end(), id_a) != tracked_pop_ids.end(), 
          "Source population is not being tracked");
      size_t id_b = pop_b.GetID();
      emp_assert(std::find(tracked_pop_ids.begin(), tracked_pop_ids.end(), id_b) != tracked_pop_ids.end(), 
          "Destination population is not being tracked");
      emp_assert(0 <= prob && prob <= 1, "Probability must be between 0 and 1");
      connections[id_a][id_b] = prob;
    }

    void AddCollectionConnection(const std::string& a, const std::string& b, double prob){
      emp_assert(emp::Has(collection_map, a), "Source collection is not tracked");
      emp_assert(emp::Has(collection_map, b), "Destination collection is not tracked");
      emp_assert(0 <= prob && prob <= 1, "Probability must be between 0 and 1");
      collection_connections[a][b] = prob;
    }

    void AddPopulationToCollection(Population& pop, const std::string& coll_name){
      size_t id = pop.GetID();
      emp_assert(std::find(tracked_pop_ids.begin(), tracked_pop_ids.end(), id) != tracked_pop_ids.end(), 
          "Cannot add population to collection: population not tracked");
      emp_assert(emp::Has(collection_map, coll_name), 
          "Cannot add population to collection, collection not tracked");
      emp::Ptr<Collection> coll_ptr = collection_map[coll_name];
      coll_ptr->Insert(pop);
    }

    size_t GetNumPatches(){
      return tracked_pop_ids.size();
    }

    Collection& GetCollection(){ return collection; }

    // Check all patches for migration, update as necessary
    size_t Update(){
      size_t num_migrations = 0;
      emp::Random & random = control.GetRandom();
      // Update populations first
      for(size_t source_pop_id : tracked_pop_ids){
        emp_assert(emp::Has(connections, source_pop_id), "Population ID was not added as key in connections map");
        Population& source_pop = control.GetPopulation(source_pop_id);
        std::unordered_map<size_t, double>& pop_conns = connections[source_pop_id];
        const size_t pop_size = source_pop.GetNumOrgs();
        if(pop_size == 0) continue;
        for(auto it = pop_conns.begin(); it != pop_conns.end(); it++){
          const size_t dest_pop_id = it->first;
          const double per_org_prob = it->second;
          const double pop_prob = 1 - emp::Pow(1 - per_org_prob, pop_size);
          if(random.P(pop_prob)){
            size_t parent_idx = random.GetUInt(source_pop.GetNumOrgs());
            Organism& org = source_pop.At(parent_idx); 
            Population& target_pop = control.GetPopulation(dest_pop_id);
            OrgPosition parent_pos(source_pop, parent_idx);
            control.DoBirth(org, parent_pos, target_pop, 1, true); 
            num_migrations++;
          }
        }
      }
      // Now do collections
      for(auto source_it = collection_map.begin(); source_it != collection_map.end(); source_it++){
        const std::string& source_name = source_it->first;
        emp::Ptr<Collection> source_ptr = source_it->second;

        emp_assert(emp::Has(collection_connections, source_name), 
            "Collection name was not added as key in connections map");

        // Make sure collection isn't empty (can't migrate if so!)
        const size_t source_size = source_ptr->GetSize();
        if(source_size == 0) continue;
        std::unordered_map<std::string, double>& conns = collection_connections[source_name];
        for(auto dest_it = conns.begin(); dest_it != conns.end(); dest_it++){
          const std::string& dest_name = dest_it->first;
          const double per_org_prob = dest_it->second;
          emp::Ptr<Collection> dest_ptr = collection_map[dest_name];
          const double pop_prob = 1 - emp::Pow(1 - per_org_prob, source_size);

          if(random.P(pop_prob)){
            size_t parent_idx = random.GetUInt(source_size);
            Organism& org = source_ptr->At(parent_idx); 
            OrgPosition parent_pos = source_ptr->GetPosAt(parent_idx);
            
            size_t target_pop_idx = random.GetUInt(dest_ptr->GetNumPops());
            Population& target_pop = *(dest_ptr->GetPop(target_pop_idx));
            control.DoBirth(org, parent_pos, target_pop, 1, true); 
            num_migrations++;
          }
        }
      }
      return num_migrations;
    }

    // Setup member functions associated with this class.
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("NUM_PATCHES",
          [](PatchPlacement & mod) { 
              return mod.GetNumPatches(); 
          }, "Get the number of patches");
      info.AddMemberFunction("ADD_PATCH",
          [](PatchPlacement & mod, Population & pop) { 
              mod.AddPatch(pop);
              return 0;
          }, "Adds an externally-managed population as a patch");
      info.AddMemberFunction("ADD_CONNECTION",
          [](PatchPlacement & mod, Population & source, Population & dest, double prob) { 
              mod.AddConnection(source, dest, prob);
              return 0;
          }, "Adds a directed, probabilistic migration connection between two patches");
      info.AddMemberFunction("UPDATE",
          [](PatchPlacement & mod) { 
              return mod.Update();
          }, "Perform probabilistic migration");
      
      info.AddMemberFunction("GET_COLLECTION",
          [](PatchPlacement & mod) { 
              return mod.GetCollection();
          }, 
          "Return a reference to the organism collection/OrgList maintained by this module");
      info.AddMemberFunction("ADD_COLLECTION",
          [](PatchPlacement & mod, Collection& coll, const std::string& name) { 
              mod.AddCollection(coll, name);
              return 0;
          }, "Adds an externally-managed collection/OrgList");
      info.AddMemberFunction("ADD_POPULATION_TO_COLLECTION",
          [](PatchPlacement & mod, Population& pop, const std::string& name) { 
              mod.AddPopulationToCollection(pop, name);
              return 0;
          }, "Inserts population into named collection/OrgList");
      info.AddMemberFunction("ADD_COLLECTION_CONNECTION",
          [](PatchPlacement & mod, const std::string& source, const std::string& dest, double prob) { 
              mod.AddCollectionConnection(source, dest, prob);
              return 0;
          }, "Adds a directed, probabilistic migration connection between two patches");
    }

  };

  MABE_REGISTER_MODULE(PatchPlacement, "Organisms are placed in well-mixed patches with migration");
}

#endif

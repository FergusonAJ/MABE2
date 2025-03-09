/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2021-2024.
 *
 *  @file  SystematicsModule.hpp
 *  @brief MABE systematic tracking module.
 */

#ifndef MABE_ANALYZE_SYSTEMATICS_MODULE_H
#define MABE_ANALYZE_SYSTEMATICS_MODULE_H

#include "../core/MABE.hpp"
#include "../core/Module.hpp"
#include "emp/Evolve/Systematics.hpp"
#include "emp/data/DataFile.hpp"

namespace mabe {

class AnalyzeSystematics : public Module {
private:

    // Systematics manager setup
    bool store_outside = false;                        ///< Track extinct non-ancestor taxa?
    bool store_ancestors = true;                       ///< Track extinct ancestor taxa?
    bool generate_output = true;                       ///< Do we generate org output?
    RequiredTraitAsString taxon_trait{this,"genome"};  ///< Which trait should taxa be based on?
    emp::Systematics <Organism, emp::String> sys;      ///< The systematics manager.

    // Output
    UpdateRange snapshot_range;            ///< Updates to start and stop snapshots + frequency.
    emp::String snapshot_file_root_name;   ///< Root name of the snapshot files.
    UpdateRange data_range;                ///< Updates to start and stop data output + frequency.
    emp::String data_file_name;            ///< Name of the data file.
    emp::DataFile data;                    ///< Data file object.

public:
    AnalyzeSystematics(mabe::MABE & control,
               const emp::String & name="AnalyzeSystematics",
               const emp::String & desc="Module to track the population's phylogeny.")
      : Module(control, name, desc)
      , sys([this](Organism& org){
              if(generate_output) org.GenerateOutput();
              return taxon_trait.Get(org);
            }, true, store_ancestors, store_outside, true)
      , snapshot_file_root_name("phylogeny")
      , data_file_name("phylogenetic_data.csv")
      , data("")
    {
      taxon_trait.SetConfigName("taxon_info");
      taxon_trait.SetConfigDesc("Trait for identification of unique taxa.");
      SetAnalyzeMod(true);    ///< Mark this module as an analyze module.
    }
    ~AnalyzeSystematics() { }

    void SetupConfig() override {
      // Settings for the systematic manager.
      LinkVar(store_outside, "store_outside", "Store all taxa that ever existed.(1 = TRUE)" );
      LinkVar(store_ancestors, "store_ancestors", "Store all ancestors of extant taxa.(1 = TRUE)" );
      LinkVar(generate_output, "generate_output", "Do we generate org output before storing?.(1 = TRUE)" );
      // Settings for output files.
      LinkVar(data_file_name, "data_file_name", "Filename for systematics data file.");
      LinkVar(snapshot_file_root_name, "snapshot_file_root_name", "Filename for snapshot files (will have update number and .csv appended to end)");
      LinkRange(snapshot_range, "snapshot_updates", "Which updates should we output a snapshot of the phylogeny?");
      LinkRange(data_range, "data_updates", "Which updates should we output a data from the phylogeny?");
    }

    void SetupModule() override {
      // Setup the data file
      data = emp::DataFile(data_file_name);
      sys.AddPhylogeneticDiversityDataNode();
      sys.AddPairwiseDistanceDataNode();
      sys.AddEvolutionaryDistinctivenessDataNode();
      std::function<size_t ()> updatefun = [this](){return control.GetUpdate();};
      data.AddFun(updatefun,"Generation", "The current generation");
      data.AddCurrent(*sys.GetDataNode("phylogenetic_diversity"), "phylogenetic_diversity","The current phylogenetic diversity.", true, true);
      data.AddStats(*sys.GetDataNode("pairwise_distance"),"pairwise_distance","pairwise distance",true,true);
      data.AddStats(*sys.GetDataNode("evolutionary_distinctiveness"),"evolutionary_distinctiveness","evolutionary distinctiveness",true,true);
      data.PrintHeaderKeys();
      data.SetTimingRange(data_range.start, data_range.step, data_range.stop);    

      // Setup the snapshot file
      std::function<emp::String(const emp::Taxon<emp::String> &)> snapshot_fun = [](const emp::Taxon<emp::String> & taxon){return taxon.GetInfo();};
      sys.AddSnapshotFun(snapshot_fun, "taxon_info", "The string representation of the information that is used to delineate what counts as a different taxon.");
    }
      
    void OnUpdate(size_t update) override {
      sys.Update();

      if (snapshot_range.IsValid(update)) {
        sys.Snapshot(snapshot_file_root_name + "_" + emp::MakeString(update) + ".csv");
      }
      data.Update(update);      
    }
    
    void TakeManualSnapshot(){
      sys.Snapshot(snapshot_file_root_name + "_manual_" + emp::MakeString(control.GetUpdate()) + ".csv");
    }

    double CheckMeanPairwiseDistance() {
      double mpd;
      if (std::isnan(sys.GetMeanPairwiseDistance())) {
          //std::cout << "Mean pairwise distance is NaN" << std::endl;
          mpd = 0;
      }
      else {
          mpd = sys.GetMeanPairwiseDistance();
      }
      return mpd;
    }
    
    static void InitType(emplode::TypeInfo & info) {
      info.AddMemberFunction("SNAPSHOT",
          [](AnalyzeSystematics & mod) { 
            mod.TakeManualSnapshot();
            return 0;
          },
          "Ouput snapshot to file");
      info.AddMemberFunction("CheckMeanPairwiseDistance",
          [](AnalyzeSystematics & mod) { 
            return mod.CheckMeanPairwiseDistance();
          },
          "Check mean pairwise distance");
    }

    void BeforeDeath(OrgPosition pos) override {
      // Notify the systematics manager when an organism dies.
      sys.RemoveOrg({pos.Pos(), (size_t)pos.PopID()});
    }

    void BeforePlacement(Organism& org, OrgPosition pos, OrgPosition ppos) override {
      // Notify the systematics manager when an organism is born.
      if (ppos.IsValid()) {
        sys.AddOrg(org, {pos.Pos(), (size_t)pos.PopID()}, {ppos.Pos(), (size_t)ppos.PopID()});
      } else {
        // We're injecting so no parent
        // Double-check that this is happening because pop is null,
        // not because parent position is illegal
        // emp_assert(ppos.PopPtr().IsNull() && "Illegal parent position");
        sys.AddOrg(org, {pos.Pos(), (size_t)pos.PopID()}, nullptr);
      }
    }

    void OnSwap(OrgPosition pos1, OrgPosition pos2) override {
      // Notify the systematics manager when an organism is moved.
      sys.SwapPositions({pos1.Pos(), (size_t)pos1.PopID()}, {pos2.Pos(), (size_t)pos2.PopID()});
    }
};

    MABE_REGISTER_MODULE(AnalyzeSystematics, "Module to track the population's phylogeny.");
}
#endif

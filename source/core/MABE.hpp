/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2024.
 *
 *  @file  MABE.hpp
 *  @brief Master controller object for a MABE run.
 *
 *  This is the master MABE controller object that hooks together all of the modules and provides
 *  the interface for them to interact.
 *
 *  MABE allows modules to interact via a set of SIGNALS that they can listen for by overriding
 *  specific base class functions.  See Module.h for a full list of available signals.
 */

#ifndef MABE_MABE_HPP
#define MABE_MABE_HPP

#include <limits>
#include <sstream>

#include "emp/base/array.hpp"
#include "emp/base/Ptr.hpp"
#include "emp/base/vector.hpp"
#include "emp/base/notify.hpp"
#include "emp/config/command_line.hpp"
#include "emp/control/Signal.hpp"
#include "emp/data/DataMap.hpp"
#include "emp/data/SimpleParser.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/math/constants.hpp"
#include "emp/math/Random.hpp"
#include "emp/tools/String.hpp"

#include "../Emplode/Emplode.hpp"

#include "Batch.hpp"
#include "Collection.hpp"
#include "data_collect.hpp"
#include "MABEBase.hpp"
#include "MABEScript.hpp"
#include "ModuleBase.hpp"
#include "Population.hpp"
#include "SigListener.hpp"
#include "TraitManager.hpp"
#include "ActionMap.hpp"

namespace mabe {

  ///  @brief The main MABE controller class
  ///
  ///  The MABE controller class manages interactions among modules, ensures that needed
  ///  components are present at startup, and triggers signals as needed.  Derived from
  ///  MABEBase, which handles all population manipulation and signal management.

  class MABE : public MABEBase {
  private:
    const emp::String VERSION = "0.0.2";

    // --- Variables to handle configuration, initialization, and error reporting ---
    bool show_help = false;      ///< Should we show "help" before exiting?
    emp::String help_topic="";   ///< What topic should we give help about?

    /// Populations used; generated in the configuration file.
    emp::vector< emp::Ptr<Population> > pops;

    emp::vector<ActionMap> action_maps;

    /// Organism pointer to use for all empty cells.
    emp::Ptr<Organism> empty_org = nullptr;

    /// Trait information to be stored on each organism.  Tracks the name, type, and current
    /// value of all traits that modules associate with organisms.
    emp::DataMap org_data_map;

    TraitManager<ModuleBase> trait_man; ///< Manage consistent read/write access to traits

    // --- Config information for command-line arguments ---
    struct ArgInfo {
      emp::String name;    ///< E.g.: "help" which would be called with "--help"
      emp::String flag;    ///< E.g.: "h" which would be called with -h
      emp::String args;    ///< Type of arguments needed: E.g.: "[filename...]"
      emp::String desc;    ///< E.g.: "Print available command-line options."

      /// Function to call when triggered.
      using fun_t = std::function<void(const emp::vector<emp::String> &)>;
      fun_t action;

      ArgInfo(const emp::String & _n, const emp::String & _f, const emp::String & _a,
              const emp::String & _d, fun_t _action)
        : name(_n), flag(_f), args(_a), desc(_d), action(_action) { }
    };

    emp::vector<ArgInfo> arg_set;              ///< Info about valid command-line arguments.
    emp::vector<emp::String> args;             ///< Command-line arguments passed in.
    emp::vector<emp::String> config_filenames; ///< Names of configuration files to load.
    emp::vector<emp::String> config_settings;  ///< Additional config commands to run.
    emp::String gen_filename;                  ///< Name of output file to generate.
    MABEScript config_script;                  ///< Configuration information for this run.
    
    // ----------- Helper Functions -----------    
    void ShowHelp();       ///< Print information on how to run the software.
    void ShowModules();    ///< List all available modules in the current compilation.
    void RunBatch();       ///< Process a whole series of MABE runs.
    void ProcessArgs();    ///< Process all arguments passed in on the command line.
    /// Setup a function as deprecated so we can phase it out.
    void Setup_CommandLine(); ///< Process all command-line args.
    void Setup_Modules();     ///< Run SetupModule() method on each module we've loaded.
    void UpdateSignals();     ///< Link signals only to modules that respond to them.

  public:
    MABE();                        ///< MABE default constructor (for testing)
    MABE(int argc, char* argv[]);  ///< MABE command-line constructor.
    MABE(const MABE &) = delete;
    MABE(MABE &&) = delete;
    ~MABE() {
      before_exit_sig.Trigger();                      // Notify modules of end...

      for (auto pop_ptr : pops) {                     // Delete all populations.
        ClearPop(*pop_ptr);
        pop_ptr.Delete();
      }
      for (auto mod_ptr : modules) mod_ptr.Delete();  // Delete all modules.
      // Delete the empty organism AFTER clearing the populations, so it's not still used.
      if (empty_org) empty_org.Delete();
    }

    /// Output args if (and only if) we are in verbose mode.
    template <typename... Ts> void Verbose(Ts &&... args) {
      if (verbose) std::cout << emp::MakeString(std::forward<Ts>(args)...) << std::endl;
    }

    void PrintAST() override {
      config_script.PrintAST();
    }

    uint64_t GetRandomSeed() const override { return random.GetSeed(); }
    void SetRandomSeed(int64_t in_seed) override {
      std::cout << "Setting Random Seed to " << in_seed << std::endl;
      random.ResetSeed(in_seed);
    }

    // --- Tools to setup runs ---
    bool Setup();

    /// Build a placeholder organism for "empty" positions in a Population
    template <typename EMPTY_MANAGER_T> void SetupEmpty();

    /// Update MABE a single time step.
    void Update(size_t num_updates=1);
    /// Update MABE one time step at a time until a stop condition is met
    void UpdateForever();

    // --- Population Management ---

    size_t GetNumPopulations() const { return pops.size(); }
    int GetPopID(std::string_view pop_name) const {
      return emp::FindEval(pops, [pop_name](const auto & p){ return p->GetName() == pop_name; });
    }
    const ActionMap & GetActionMap(size_t id) const { return action_maps[id]; }
    ActionMap & GetActionMap(size_t id) { return action_maps[id]; }

    size_t GetPopulationID(const emp::String & name) const {
      for (size_t id = 0; id < pops.size(); ++id) {
        if (pops[id]->GetName() == name) return id;
      }
      return emp::MAX_SIZE_T;
    }
    Population & GetPopulation(size_t id) { return *pops[id]; }
    const Population & GetPopulation(size_t id) const { return *pops[id]; }

    Population & GetPopulation(const emp::String & name) {
      return *pops[GetPopulationID(name)];
    }
    const Population & GetPopulation(const emp::String & name) const {
      return *pops[GetPopulationID(name)];
    }


    Population & AddPopulation(const emp::String & name, size_t pop_size=0) override;

    /// Move an organism from one position to another; kill anything that previously occupied
    /// the target position.
    void MoveOrg(OrgPosition from_pos, OrgPosition to_pos) {
      if (from_pos != to_pos) {
        ClearOrgAt(to_pos);
        SwapOrgs(from_pos, to_pos);
      }
    }

    /// Inject one or more copies of an organism and return the positions they were placed in.
    Collection Inject(Population & pop, const Organism & org, size_t copy_count=1);

    /// Inject this specific instance of an organism and turn over the pointer to be managed
    /// by MABE.  Return the position the organism was placed in.
    OrgPosition InjectInstance(Population & pop, emp::Ptr<Organism> org_ptr);
    
    /// Inject given organism (no parent) at the specified place in the population
    /// Also turn the pointer over to the population to be managed. 
    /// Return the same position.
    OrgPosition InjectInstanceAt(
      Population & pop, 
      emp::Ptr<Organism> org_ptr, 
      OrgPosition pos);
    
    /// Add one or more organisms of a specified type (provide the type name; the MABE controller
    /// will create instances of it.)  Returns the positions the organisms were placed.
    Collection Inject(Population & pop, const emp::String & type_name, size_t copy_count=1);
   
    /// Injects N organisms with the given genome 
    Collection InjectGenome(
        Population & pop, 
        const emp::String & type_name, 
        const emp::String & genome, 
        size_t copy_count=1);
  
    /// Add an organsim of a specified type and genome to the specific world location
    OrgPosition InjectGenomeAt(
      Population & pop, 
      const emp::String & type_name, 
      const emp::String& genome, 
      OrgPosition pos);

    /// Add an organism of a specified type and population (provide names of both and they
    /// will be properly setup.)
    Collection InjectByName(const emp::String & pop_name,
                             const emp::String & type_name, 
                             size_t copy_count=1);

    /// Inject a copy of the provided organism at a specified position.
    void InjectAt(const Organism & org, OrgPosition pos) {
      emp_assert(pos.IsValid());
      emp::Ptr<Organism> inject_org = org.CloneOrganism();
      on_inject_ready_sig.Trigger(*inject_org, GetPopulation(pos.PopID()));
      AddOrgAt( inject_org, pos);
    }

    /// Write out the current population to a stream
    bool SavePopulation(Population & pop, std::ostream& os);

    /// Write out the current population to file
    bool SavePopulationToFile(Population & pop, const emp::String& filename){
      std::ofstream ofs(filename);
      return SavePopulation(pop, ofs);
    }
   
    // Load a whole population from a given file
    Collection LoadPopulationFromFile(
        Population & pop, 
        const emp::String& org_type_name,
        const emp::String& filename);

    /// Give birth to one or more offspring; return position of last placed.
    /// Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
    /// Regular signal triggers occur in AddOrgAt.
    Collection DoBirth(const Organism & org,
                       OrgPosition ppos,
                       Population & target_pop,
                       size_t birth_count=1,
                       bool do_mutations=true);

    Collection DoBirth(const Organism & org,
                       OrgPosition ppos,
                       OrgPosition target_pos,
                       bool do_mutations=true);


    /// A shortcut to DoBirth where only the parent position needs to be supplied;
    /// Return all offspring placed.
    Collection Replicate(OrgPosition ppos, Population & target_pop,
                          size_t birth_count=1, bool do_mutations=true) {
      return DoBirth(*ppos, ppos, target_pop, birth_count, do_mutations);
    }

    /// Remove all organisms from a population; does not change size.
    void ClearPop(Population & pop) {
      for (PopIterator pos = pop.begin(); pos != pop.end(); ++pos) ClearOrgAt(pos);
    }

    /// Resize a population while clearing all of the organisms in it.
    void EmptyPop(Population & pop, size_t new_size=0) {
      ClearPop(pop);
      MABEBase::ResizePop(pop, new_size);
    }

    /// Copy all of the organisms into a new population (clearing orgs already there)
    void CopyPop(const Population & from_pop, Population & to_pop) override {
      EmptyPop(to_pop, from_pop.GetSize()); 
      for (size_t pos=0; pos < from_pop.GetSize(); ++pos) {
        if (from_pop.IsEmpty(pos)) continue;
        InjectAt(from_pop[pos], to_pop.IteratorAt(pos));
      }
    }

    /// Move all organisms from one population to another.
    void MoveOrgs(Population & from_pop, Population & to_pop, bool reset_to) override;

    /// Return a random position from a designated population.
    OrgPosition GetRandomPos(Population & pop) {
      emp_assert(pop.GetSize() > 0);
      return pop.IteratorAt( random.GetUInt(pop.GetSize()) );
    }

    /// Return a random position from the population with the specified id.
    OrgPosition GetRandomPos(size_t pop_id) { return GetRandomPos(GetPopulation(pop_id)); }

    /// Return a random position from a designated population with a living organism in it.
    OrgPosition GetRandomOrgPos(Population & pop);

    /// Return a random position of a living organism from the population with the specified id.
    OrgPosition GetRandomOrgPos(size_t pop_id) { return GetRandomOrgPos(GetPopulation(pop_id)); }


    // --- Collection Management ---

    emp::String ToString(const mabe::Collection & collect) const { return collect.ToString(); }
    Collection ToCollection(const emp::String & load_str);

    Collection GetAlivePopulation(size_t id) {
      Collection col(GetPopulation(id));
      col.RemoveEmpty();
      return col;
    }


    // --- Module Management ---

    /// Get the unique id of a module with the specified name.
    int GetModuleID(const emp::String & mod_name) const {
      return emp::FindEval(modules, [mod_name](const auto & m){ return m->GetName() == mod_name; });
    }

    /// Get a reference to a module with the specified ID.
    const ModuleBase & GetModule(int id) const { return *modules[(size_t) id]; }
    ModuleBase & GetModule(int id) { return *modules[(size_t) id]; }

    /// Get a reference to a module with the specified name.
    const ModuleBase & GetModule(const emp::String & mod_name) const {
      return *modules[(size_t) GetModuleID(mod_name)];
    }
    ModuleBase & GetModule(const emp::String & mod_name) {
      return *modules[(size_t) GetModuleID(mod_name)];
    }

    /// Add a new module of the specified type.
    template <typename MOD_T, typename... ARGS>
    MOD_T & AddModule(ARGS &&... args) {
      auto new_mod = emp::NewPtr<MOD_T>(*this, std::forward<ARGS>(args)...);
      modules.push_back(new_mod);
      return *new_mod;
    }

    // --- Deal with Organism TRAITS ---
    TraitManager<ModuleBase> & GetTraitManager() { return trait_man; }

    /// Resets ALL traits for a given organism to their default values
    void ResetTraits(Organism& org){
      trait_man.ResetAll(org.GetDataMap());
    }

    /// Return the DataMap for organisms
    emp::DataMap GetOrganismDataMap(){
      return org_data_map;
    }

    /// Build a lambda function that takes an organism applied the provided equation to it.
    /// (The provided data layout must match that or the organisms.)
    auto BuildTraitEquation(const emp::DataLayout & data_layout, const emp::String & equation)
    {
      return config_script.BuildTraitEquation(data_layout, equation);
    }
    
    void Setup_Traits();   ///< Load organism traits and test for module conflicts.    

    /// Build a trait equations for organisms in a given population.
    auto BuildTraitEquation(const Population & pop, const emp::String & equation) {
      return BuildTraitEquation(pop.GetDataLayout(), equation);
    }

    const std::set<emp::String> & GetEquationTraits(const emp::String & equation) {
      return config_script.GetEquationTraits(equation);
    }

    MABEScript& GetConfigScript() { return config_script; }

    // Pass anything we want to load into the config script.
    template <typename... Ts>
    void Load(Ts &&... args) { config_script.Load(std::forward<Ts>(args)...); }

    // Execute a single statement (and return a value-based result)
    emp::Datum Execute(const emp::String & cmd) { return config_script.Execute(cmd); }

    bool OK();           ///< Sanity checks for debugging

    // Checks for which modules are actively being triggered.
    using mod_ptr_t = emp::Ptr<ModuleBase>;
    bool BeforeUpdate_IsTriggered(mod_ptr_t mod) { return before_update_sig.cur_mod == mod; };
    bool OnUpdate_IsTriggered(mod_ptr_t mod) { return on_update_sig.cur_mod == mod; };
    bool BeforeRepro_IsTriggered(mod_ptr_t mod) { return before_repro_sig.cur_mod == mod; };
    bool OnOffspringReady_IsTriggered(mod_ptr_t mod) { return on_offspring_ready_sig.cur_mod == mod; };
    bool OnInjectReady_IsTriggered(mod_ptr_t mod) { return on_inject_ready_sig.cur_mod == mod; };
    bool BeforePlacement_IsTriggered(mod_ptr_t mod) { return before_placement_sig.cur_mod == mod; };
    bool OnPlacement_IsTriggered(mod_ptr_t mod) { return on_placement_sig.cur_mod == mod; };
    bool BeforeMutate_IsTriggered(mod_ptr_t mod) { return before_mutate_sig.cur_mod == mod; };
    bool OnMutate_IsTriggered(mod_ptr_t mod) { return on_mutate_sig.cur_mod == mod; };
    bool BeforeDeath_IsTriggered(mod_ptr_t mod) { return before_death_sig.cur_mod == mod; };
    bool BeforeSwap_IsTriggered(mod_ptr_t mod) { return before_swap_sig.cur_mod == mod; };
    bool OnSwap_IsTriggered(mod_ptr_t mod) { return on_swap_sig.cur_mod == mod; };
    bool BeforePopResize_IsTriggered(mod_ptr_t mod) { return before_pop_resize_sig.cur_mod == mod; };
    bool OnPopResize_IsTriggered(mod_ptr_t mod) { return on_pop_resize_sig.cur_mod == mod; };
    bool BeforeExit_IsTriggered(mod_ptr_t mod) { return before_exit_sig.cur_mod == mod; };
    bool OnHelp_IsTriggered(mod_ptr_t mod) { return on_help_sig.cur_mod == mod; };
  };


  // ========================== OUT-OF-CLASS DEFINITIONS! ==========================

  // ---------------- PRIVATE MEMBER FUNCTIONS -----------------

  /// Print information on how to run the software.
  void MABE::ShowHelp() {
    std::cout << "MABE v" << VERSION << "\n";
    on_help_sig.Trigger();

    if (help_topic == "") {
      std::cout << "Usage: " << args[0] << " [options]\n"
                << "Options:\n";
      for (const auto & cur_arg : arg_set) {
        std::cout << "  " << cur_arg.flag << " " << cur_arg.args
                  << " : " << cur_arg.desc << " (or " << cur_arg.name << ")"
                  << std::endl;
      }
    }
    else {
      auto & mod_map = GetModuleMap();
      std::cout << "TOPIC: " << help_topic << std::endl;
      if (emp::Has(mod_map, help_topic)) {
        const auto & info = mod_map[help_topic];
        std::cout << "\n--- MABE Module ---\n\n"
                  << "Description:\n";
        for (const emp::String & line : info.full_desc) {
          std::cout << "  " << line << "\n";
        }
        std::cout << "\nDefault Configuration:\n\n";

        // Print a configuration template for the user.
        emp::String config_code =info.name + " example_module;";
        config_script.LoadStatements(config_code, "help_output");
        config_script.WriteSymbol("example_module", std::cout, "  ");
      }
      else {
        std::cout << "Unknown keyword.\n";
      }

    }
    exit_now = true;
  }

  /// List all of the available modules included in the current compilation.
  void MABE::ShowModules() {
    std::cout << "MABE v" << VERSION << "\n"
              << "Active modules:\n";
    // for (auto mod_ptr : modules) {
    //   std::cout << "  " << mod_ptr->GetName() << " : " << mod_ptr->GetDesc() << "\n";
    // }          
    std::cout << "Available modules:\n";
    for (auto & [type_name,mod] : GetModuleMap()) {
      std::cout << "  " << type_name << " : " << mod.brief_desc << "\n";
    }          
    exit_now = true;;
  }


  void MABE::RunBatch() {
    exit_now = true;  // Exit after running the batch of files.

    if (config_filenames.size() == 0) {
      std::cout << "Must specify name of batch file to run." << std::endl;
      return;
    }
    if (config_filenames.size() > 1) {
      std::cout << "Only one batch file may be specified." << std::endl;
      for (size_t i = 1; i < config_filenames.size(); ++i) {
        std::cout << "...ignoring '" << config_filenames[i] << "'" << std::endl;
      }
    }

    mabe::Batch batch(config_filenames[0], args[0]);
    batch.Process();
    batch.Run();
  }

  void MABE::ProcessArgs() {
    arg_set.emplace_back("--batch", "-b",    "[filename]    ", "Process a full batch of runs",
      [this](const emp::vector<emp::String> & in){ config_filenames = in; RunBatch(); } );    
    arg_set.emplace_back("--filename", "-f", "[filename...] ", "Filenames of configuration settings",
      [this](const emp::vector<emp::String> & in){ config_filenames = in; } );
    arg_set.emplace_back("--generate", "-g", "[filename]    ", "Generate a new output file",
      [this](const emp::vector<emp::String> & in) {
        if (in.size() != 1) {
          std::cout << "'--generate' must be followed by a single filename.\n";
          exit_now = true;
        } else {
          // MABE config files can be generated FROM a *.gen file, typically creating a *.mabe
          // file.  If output file is *.gen assume an error. (for now; override should be allowed)
          if (in[0].size() > 4 && in[0].substr(in[0].size()-4) == ".gen") {
            emp::notify::Error("Generated file ", in[0],
                               " not allowed to be *.gen; typically should end in *.mabe.");
            exit_now = true;
          }
          else gen_filename = in[0];
        }
      });
    arg_set.emplace_back("--help", "-h", "              ", "Help; print command-line options for MABE",
      [this](const emp::vector<emp::String> & in){
        show_help = true;
        if (in.size()) help_topic = in[0];
      });
    arg_set.emplace_back("--modules", "-m", "              ", "Module list",
      [this](const emp::vector<emp::String> &){ ShowModules(); } );
    arg_set.emplace_back("--set", "-s", "[param=value] ", "Set specified parameter",
      [this](const emp::vector<emp::String> & in){
        std::cout << "Adding command-line setting:";
        for (const emp::String & x : in) std::cout << " " << x;
        std::cout << std::endl;
        emp::Append(config_settings, in);
        config_settings.push_back(";"); // Extra semi-colon so not needed on command line.
      });
    arg_set.emplace_back("--version", "-v", "              ", "Version ID of MABE",
      [this](const emp::vector<emp::String> &){
        std::cout << "MABE v" << VERSION << "\n";
        exit_now = true;
      });
    arg_set.emplace_back("--verbose", "-+", "              ", "Output extra setup info",
      [this](const emp::vector<emp::String> &){ verbose = true; } );

    // Scan through all input argument positions.
    for (size_t pos = 1; pos < args.size(); pos++) {
      // Match the input argument to the function to call.
      bool found = false;
      for (auto & cur_arg : arg_set) {
        // If we have a match...
        if (args[pos] == cur_arg.name || args[pos] == cur_arg.flag) {
          // ...collect all of the options associated with this match.
          emp::vector<emp::String> option_args;
          // We want args until we run out or hit another option.
          while (pos+1 < args.size() && args[pos+1][0] != '-') {
            option_args.push_back(args[++pos]);
          }

          // And call the function!
          cur_arg.action(option_args);
          found = true;
          break;            
        }
      }
      if (found == false) {
        emp::notify::Message("Error: unknown command line argument '", args[pos], "'.");
        show_help = true;
        break;
      }
    }

    if (show_help) ShowHelp();
  }

  void MABE::Setup_CommandLine() {
    ProcessArgs();         // Deal with command-line inputs.
    if (exit_now) return;  // If command-line arguments require exit (e.g., after '--help')

    // If filenames have been specified on command line, load each in order.
    if (config_filenames.size()) {
      std::cout << "Loading file(s): " << emp::MakeQuotedList(config_filenames) << std::endl;
      config_script.Load(config_filenames);   // Load files
    }

    // If other variable settings have been specified, run them AFTER files are loaded.
    if (config_settings.size()) {
      std::cout << "Loading command-line settings." << std::endl;
      config_script.LoadStatements(config_settings, "command-line settings");      
    }

    // If we are writing a file, do so and then exit.
    if (gen_filename != "") {
      std::cout << "Generating file '" << gen_filename << "'." << std::endl;
      config_script.Write(gen_filename);
      exit_now = true;
    }    
  }

  /// As part of the main Setup(), run SetupModule() method on each module we've loaded.
  void MABE::Setup_Modules() {
    trait_man.Unlock(); // Allow traits to be linked.

    // Allow the user-defined module SetupModule() member functions run.  These are
    // typically used for any internal setup needed by modules are the configuration is
    // complete.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      mod_ptr->SetupModule_Internal();
      mod_ptr->SetupModule();
    }
  }

  /// As part of the main Setup(), load in all of the organism traits that modules need to
  /// read or write and make sure that there aren't any conflicts.
  void MABE::Setup_Traits() {
    Verbose("Analyzing configuration of ", trait_man.GetSize(), " traits.");

    trait_man.Verify(verbose);            // Make sure modules are accessing traits consistently
    trait_man.RegisterAll(org_data_map);  // Load in all of the traits to the DataMap
    org_data_map.LockLayout();            // Freeze the data map into its current state

    // Alert modules (especially org managers) to the final set of traits.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      mod_ptr->SetupDataMap_Internal(org_data_map); // Internal indication that DataMap is locked in.
      mod_ptr->SetupDataMap(org_data_map);    // User-available indication that DataMap is locked in.
    }
  }

  /// Link signals to the modules that implement responses to those signals.
  void MABE::UpdateSignals() {
    // Clear all module vectors.
    for (auto modv : sig_ptrs) modv->resize(0);

    // Loop through each module to update its signals.
    for (emp::Ptr<ModuleBase> mod_ptr : modules) {
      // For the current module, loop through all of the signals.
      for (size_t sig_id = 0; sig_id < sig_ptrs.size(); sig_id++) {
        if (mod_ptr->has_signal[sig_id]) sig_ptrs[sig_id]->push_back(mod_ptr);
      }
    }

    // Now that we have scanned the signals, we can turn off the re-scan flag.
    rescan_signals = false;
  }


  // ---------------- PUBLIC MEMBER FUNCTIONS -----------------


  MABE::MABE() : config_script(*this)
  {
    // Updates to scripting language that require full controller functionality.

    // 'INJECT' allows a user to add an organism to a population; 
    //    returns collection of added orgs.
    emplode::TypeInfo & pop_type = config_script.GetType("Population");
    std::function<Collection(Population &, const emp::String &, size_t)> inject_fun =
      [this](Population & pop, const emp::String & org_type_name, size_t count) {
        return Inject(pop, org_type_name, count);
      };
    pop_type.AddMemberFunction("INJECT", inject_fun,
      "Inject organisms into population.  "
      "Args: org_name, org_count; Return: OrgList of injected orgs.");
    // 'INJECT_GENOME' allows a user to add organisms with a specific genome to a population;
    //    returns collection of added orgs.
    std::function<Collection(Population &, const emp::String &, 
        const emp::String &, size_t)> inject_genome_fun =
      [this](Population & pop, const emp::String & org_type_name, 
          const emp::String & genome, size_t count) {
        return InjectGenome(pop, org_type_name, genome, count);
      };
    pop_type.AddMemberFunction("INJECT_GENOME", inject_genome_fun,
      "Inject organisms with a given genome into population.  "
      "Args: org_name, genome, org_count; Return: OrgList of injected orgs.");
    pop_type.AddMemberFunction("SAVE_TO_FILE", 
        [this](Population & pop, const emp::String& filename){
          return SavePopulationToFile(pop, filename);
        }, 
        "Save the population to the given file. " 
        "Args: pop, filename; Return: Success boolean");
    
    pop_type.AddMemberFunction("LOAD_FROM_FILE", 
        [this](Population & pop, 
            const emp::String& org_type_name, 
            const emp::String& filename){
          return LoadPopulationFromFile(pop, org_type_name, filename);
        }, 
        "Load the population from the given file. " 
        "Args: pop, org_type, filename; Return: Collection of orgs added");

    pop_type.AddMemberFunction("CLEAR", 
        [this](Population& pop){
          ResizePop(pop, 0);
          return 0;
        }, 
        "Removes all organisms from the population. No args.");

    // Setup all known modules as available types in the config file.
    for (auto & [type_name,mod] : GetModuleMap()) {
      auto mod_init_fun = [this,mod=&mod](const emp::String & name) -> emp::Ptr<emplode::EmplodeType> {
        return mod->obj_init_fun(*this,name);
      };
      auto & type_info = config_script.AddType(type_name, mod.brief_desc, mod_init_fun, nullptr, mod.type_id);
      mod.type_init_fun(type_info);  // Setup functions for this module.
    }

    // Default the list of arguments to the (likely) name of the executable.
    args.push_back("MABE");
  }

  MABE::MABE(int argc, char* argv[]) : MABE()
  {
    args = emp::cl::ArgsToStrings(argc, argv);
  }

  bool MABE::Setup() {
    // Read in command line arguments, respond to flags, load associated files, and deal with
    // any other command-line settings.
    Setup_CommandLine(); 
    if (exit_now) return false;  // If any of the inital flags triggered an 'exit_now', do so.

    Setup_Modules();    // Run SetupModule() on each module for linking traits or other setup.
    Setup_Traits();     // Make sure module traits do not clash.
    UpdateSignals();    // Setup the appropriate modules to be linked with each signal.
    SetupBase();        // Call Setup on MABEBase (which will report errors)

    return true;
  }

  /// Update MABE world.
  void MABE::Update(size_t num_updates) {
    if (update == 0) config_script.Trigger("START");

    const size_t target_update = update + num_updates;
    while (update < target_update && !exit_now) {
      emp_assert(OK(), update);                 // In debug mode, keep checking MABE integrity
      if (rescan_signals) UpdateSignals();      // If we have reason to, update module signals
      before_update_sig.Trigger(update);        // Signal that a new update is about to begin
      update++;                                 // Increment 'update' to start new update
      on_update_sig.Trigger(update);            // Signal all modules about the new update
      config_script.Trigger("UPDATE", update);  // Trigger any updated-based events
    }
  }
  
  /// Update MABE world until a stop condition is met
  void MABE::UpdateForever() {
    if (update == 0) config_script.Trigger("START");
    while (!exit_now) {
      emp_assert(OK(), update);                 // In debug mode, keep checking MABE integrity
      if (rescan_signals) UpdateSignals();      // If we have reason to, update module signals
      before_update_sig.Trigger(update);        // Signal that a new update is about to begin
      update++;                                 // Increment 'update' to start new update
      on_update_sig.Trigger(update);            // Signal all modules about the new update
      config_script.Trigger("UPDATE", update);  // Trigger any updated-based events
    }
  }

  /// Setup an organism as a placeholder for all "empty" positions in the population.
  template <typename EMPTY_MANAGER_T>
  void MABE::SetupEmpty() {
    if (empty_org) empty_org.Delete();  // If we already have an empty organism, replace it.
    auto & empty_manager =
      AddModule<EMPTY_MANAGER_T>("EmptyOrg", "Manager for all 'empty' organisms in any population.");
    empty_manager.SetBuiltIn();         // Don't write the empty manager to config.

    empty_org = empty_manager.template Make<Organism>();
  }

  /// New populations must be given a name and an optional size.
  Population & MABE::AddPopulation(const emp::String & name, size_t pop_size) {
    int pop_id = (int) pops.size();
      emp::Ptr<Population> new_pop = emp::NewPtr<Population>(name, pop_id, pop_size, empty_org);
      pops.push_back(new_pop);

      // Setup default placement functions for the new population.
      new_pop->SetPlaceBirthFun( [this,new_pop](Organism & /*org*/, OrgPosition /*ppos*/) {
        return PushEmpty(*new_pop);
      });
      new_pop->SetPlaceInjectFun( [this,new_pop](Organism & /*org*/) {
        return PushEmpty(*new_pop);
      });
      new_pop->SetFindNeighborFun( [this,new_pop](OrgPosition pos) {
        if (pos.IsInPop(*new_pop)) return OrgPosition(); // Wrong pop!  No neighbor.
        // Return a random org since no structure to population.
        return OrgPosition(new_pop, GetRandom().GetUInt(new_pop->GetSize()));
      });
      action_maps.emplace_back();

      return *new_pop;
    }

  /// Inject a copy of the provided organism and return the position it was placed in;
  /// if more than one is added, return the position of the final injection.
  Collection MABE::Inject(Population & pop, const Organism & org, size_t copy_count) {
    emp_assert(org.GetDataMap().SameLayout(org_data_map));
    Collection placement_set;
    for (size_t i = 0; i < copy_count; i++) {
      emp::Ptr<Organism> inject_org = org.CloneOrganism();
      on_inject_ready_sig.Trigger(*inject_org, pop);
      OrgPosition pos = pop.PlaceInject(*inject_org);
      if (pos.IsValid()) {
        AddOrgAt( inject_org, pos);
        placement_set.Insert(pos);
      } else {
        inject_org.Delete();          
        emp::notify::Error("Invalid position; failed to inject organism ", i, "!");
      }
    }
    return placement_set;
  }

  /// Inject given organism (no parent) at the specified place in the population
  /// Also turn the pointer over to the population to be managed. 
  /// Return the same position.
  OrgPosition MABE::InjectInstanceAt(
      Population & pop, 
      emp::Ptr<Organism> org_ptr, 
      OrgPosition pos){
    emp_assert(org_ptr->GetDataMap().SameLayout(org_data_map));
    on_inject_ready_sig.Trigger(*org_ptr, pop);
    if (pos.IsValid()) AddOrgAt( org_ptr, pos);
    else {
      org_ptr.Delete();          
      emp::notify::Error("Invalid position; failed to inject organism!");
    }
    return pos;
  }

  /// Inject this specific instance of an organism and turn over the pointer to be managed
  /// by MABE.  Return the position the organism was placed in.
  OrgPosition MABE::InjectInstance(Population & pop, emp::Ptr<Organism> org_ptr) {
    emp_assert(org_ptr->GetDataMap().SameLayout(org_data_map));
    on_inject_ready_sig.Trigger(*org_ptr, pop);
    OrgPosition pos = pop.PlaceInject(*org_ptr);
    if (pos.IsValid()) AddOrgAt( org_ptr, pos);
    else {
      org_ptr.Delete();          
      emp::notify::Error("Invalid position; failed to inject organism!");
    }
    return pos;
  }
  

  /// Add an organism of a specified type to the world (provide the type name and the
  /// MABE controller will create instances of it.)  Returns the position of the last
  /// organism placed.
  Collection MABE::Inject(Population & pop, const emp::String & type_name, size_t copy_count) {
    Verbose("Injecting ", copy_count, " orgs of type '", type_name,
            "' into population ", pop.GetID());

    auto & org_manager = GetModule(type_name);            // Look up type of organism.
    Collection placement_set;                             // Track set of positions placed.
    for (size_t i = 0; i < copy_count; i++) {             // Loop through, injecting each instance.
      auto org_ptr = org_manager.Make<Organism>(random);  // ...Build an org of this type.
      OrgPosition pos = InjectInstance(pop, org_ptr);     // ...Inject it into the population.
      placement_set.Insert(pos);                          // ...Record the position.
    }

    return placement_set;                                 // Return last position injected.
  }

  /// Add an organsim of a specified type and genome to the specific world location
  OrgPosition MABE::InjectGenomeAt(
      Population & pop, 
      const emp::String & type_name, 
      const emp::String& genome, 
      OrgPosition pos) {
    auto & org_manager = GetModule(type_name);            // Look up type of organism.
    Collection placement_set;                             // Track set of positions placed.
    auto org_ptr = org_manager.Make<Organism>(random);  // ...Build an org of this type.
    org_ptr->GenomeFromString(genome);
    return InjectInstanceAt(pop, org_ptr, pos);
  }

  /// Add an organsim of a specified type and genome to the world (provide the type name and 
  /// the MABE controller will create instances of it.)  Returns the position of the last
  /// organism placed.
  Collection MABE::InjectGenome(
      Population & pop, 
      const emp::String & type_name, 
      const emp::String& genome, 
      size_t copy_count) {
    Verbose("Injecting ", copy_count, " orgs of type '", type_name, 
            "' with genome '", genome,
            "' into population ", pop.GetID());

    auto & org_manager = GetModule(type_name);            // Look up type of organism.
    Collection placement_set;                             // Track set of positions placed.
    auto org_ptr = org_manager.Make<Organism>(random);  // ...Build an org of this type.
    org_ptr->GenomeFromString(genome);
    for(size_t org_idx = 0; org_idx < copy_count; org_idx++){
      emp::Ptr<Organism> inject_org = org_ptr->CloneOrganism();
      OrgPosition inject_pos = InjectInstance(pop, inject_org);
      placement_set.Insert(inject_pos);
    }
    org_ptr.Delete();
    return placement_set;
  }

  /// Add an organism of a specified type and population (provide names of both and they
  /// will be properly setup.)
  Collection MABE::InjectByName(const emp::String & pop_name,
                                const emp::String & type_name, 
                                size_t copy_count) {      
    int pop_id = GetPopID(pop_name);
    if (pop_id == -1) {
      emp::notify::Error("Invalid population name used in inject: ",
                         "org_type= '", type_name, "'; ",
                         "pop_name= '", pop_name, "'; ",
                         "copy_count=", copy_count);
    }
    Population & pop = GetPopulation(pop_id);
    return Inject(pop, type_name, copy_count); // Inject the organisms.
  }
    
  /// Write out the current population to a stream
  bool MABE::SavePopulation(Population & pop, std::ostream& os){
    for(auto org_it = pop.begin(); org_it != pop.end(); org_it++){
      if(org_it.IsEmpty()) std::cout << "<<EMPTY>>\n";
      else os << (*org_it).ToString() << "\n";
    }
    os.flush();
    return true;
  }
    
  // Load a whole population from a file
  Collection MABE::LoadPopulationFromFile(
      Population & pop, 
      const emp::String& org_type_name,
      const emp::String& filename){
    Collection placement_set;                             // Track set of positions placed.
    emp::File file(filename);
    for(size_t org_idx = 0; org_idx < file.GetNumLines(); ++org_idx){
      OrgPosition pos = pop.PlaceInject(*empty_org);
      if(file[org_idx] != "<<EMPTY>>" && file[org_idx].size() > 0){
        InjectGenomeAt(pop, org_type_name, file[org_idx], pos);
        placement_set.Insert(pos);                          // ...Record the position.
      }
    }
    return placement_set;
  }

  /// Give birth to one or more offspring; return position of last placed.
  /// Triggers 'before repro' signal on parent (once) and 'offspring ready' on each offspring.
  /// Regular signal triggers occur in AddOrgAt.
  Collection MABE::DoBirth(const Organism & org,
                           OrgPosition ppos,
                           Population & target_pop,
                           size_t birth_count,
                           bool do_mutations) {
    emp_assert(org.IsEmpty() == false);             // Empty cells cannot reproduce.
    before_repro_sig.Trigger(ppos);                 // Signal reproduction event.
    config_script.Trigger("BEFORE_REPRO");          // Trigger any updated-based events
    OrgPosition pos;                                // Position of each offspring placed.
    emp::Ptr<Organism> new_org;
    Collection birth_list;                          // Track positions of all offspring.
    for (size_t i = 0; i < birth_count; i++) {      // Loop through offspring, adding each
      new_org = do_mutations ? org.MakeOffspringOrganism(random) : org.CloneOrganism();

      // Alert modules that offspring is ready, then find its birth position.
      on_offspring_ready_sig.Trigger(*new_org, ppos, target_pop);
      pos = target_pop.PlaceBirth(*new_org, ppos);

      // If this placement is valid, do so.  Otherwise delete the organism.
      if (pos.IsValid()) {
        AddOrgAt(new_org, pos, ppos);
        birth_list.Insert(pos);
      }
      else new_org.Delete();
    }
    return birth_list;
  }

  Collection MABE::DoBirth(const Organism & org,
                           OrgPosition ppos,
                           OrgPosition target_pos,
                           bool do_mutations) {
    emp_assert(org.IsEmpty() == false);  // Empty cells cannot reproduce.
    emp_assert(target_pos.IsValid());    // Target positions must already be valid.

    before_repro_sig.Trigger(ppos);
    config_script.Trigger("BEFORE_REPRO");  // Trigger any updated-based events
    emp::Ptr<Organism> new_org = do_mutations ? org.MakeOffspringOrganism(random) : org.CloneOrganism();
    on_offspring_ready_sig.Trigger(*new_org, ppos, target_pos.Pop());

    AddOrgAt(new_org, target_pos, ppos);

    return target_pos;
  }

  void MABE::MoveOrgs(Population & from_pop, Population & to_pop, bool reset_to) {
    // Get the starting point for the new organisms to ove to.
    Population::iterator_t it_to = reset_to ? to_pop.begin() : to_pop.end();

    // Prepare the "to" population before moving the new organisms in.
    if (reset_to) EmptyPop(to_pop, from_pop.GetSize());   // Clear out the population.
    else ResizePop(to_pop, to_pop.GetSize() + from_pop.GetSize());

    // Move the organisms over
    for (auto it_from = from_pop.begin(); it_from != from_pop.end(); ++it_from, ++it_to) {
      if (it_from.IsOccupied()) MoveOrg(it_from, it_to);
    }

    // Clear out the from population now that we're done with it.
    EmptyPop(from_pop, 0);
  }

  /// Return a random position from a designated population with a living organism in it.
  OrgPosition MABE::GetRandomOrgPos(Population & pop) {
    emp_assert(pop.GetNumOrgs() > 0, "GetRandomOrgPos cannot be called if there are no orgs.");
    OrgPosition pos = GetRandomPos(pop);
    while (pos.IsEmpty()) pos = GetRandomPos(pop);
    return pos;
  }


  // --- Collection Management ---

  Collection MABE::ToCollection(const emp::String & load_str) {
    Collection out;
    auto slices = emp::view_slices(load_str, ',');
    for (auto name : slices) {
      int pop_id = GetPopID(name);
      if (pop_id == -1) emp::notify::Error("Unknown population: ", name);
      else out.Insert(GetPopulation(pop_id));
    }
    return out;
  }

  bool MABE::OK() {
    bool result = true;
    for (auto mod_ptr : modules) result &= mod_ptr->OK(); // Ensure modules are okay.
    for (auto pop_ptr : pops) result &= pop_ptr->OK();    // Ensure populations are okay.
    return result;
  }

}

#endif

/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2024.
 *
 *  @file  SignalGPOrg.hpp
 *  @brief An organism consisting of a SignalGP program that is run on virtual hardware
 *  @note Status: ALPHA
 */

#ifndef MABE_SIGNAL_GP_ORGANISM_H
#define MABE_SIGNAL_GP_ORGANISM_H

#include "../core/MABE.hpp"
#include "../core/Organism.hpp"
#include "../core/OrganismManager.hpp"

#include "emp/bits/BitVector.hpp"
#include "emp/math/Distribution.hpp"
#include "emp/math/random_utils.hpp"

#include "sgp/cpu/LinearFunctionsProgramCPU.hpp"
#include "sgp/cpu/mem/BasicMemoryModel.hpp"

#include "sgp/inst/lfpbm/InstructionAdder.hpp"
#include "sgp/inst/lfpbm/inst_impls.hpp"

#include "MutatorLinearFunctionsProgram.hpp"

namespace mabe {

  class SignalGPOrg : public OrganismTemplate<SignalGPOrg> {
  public: 
    using mem_model_t = sgp::cpu::mem::BasicMemoryModel;
    using hardware_t = sgp::cpu::LinearFunctionsProgramCPU<mem_model_t>;
    using inst_lib_t = typename hardware_t::inst_lib_t;
    using event_lib_t = typename hardware_t::event_lib_t;
    using program_t = typename hardware_t::program_t;
    using mutator_t = psynth::MutatorLinearFunctionsProgram<hardware_t, hardware_t::tag_t, hardware_t::arg_t>;

  protected:
    emp::BitVector bits;
    program_t program;

    void InitializeHardware(){
      // Add instructions to the instruction library
      sgp::inst::lfpbm::InstructionAdder<hardware_t> inst_directory;
      inst_directory.AddAllDefaultInstructions(SharedData().inst_lib);
      // Initialize the actual hardware
      emp::Random & rand = GetManager().GetControl().GetRandom();
      SharedData().hardware_ptr = emp::NewPtr<hardware_t>(
          rand, 
          SharedData().inst_lib, 
          SharedData().event_lib
      );
    }

    void InitializeMutator(){
      SharedData().mutator_ptr = emp::NewPtr<mutator_t>(SharedData().inst_lib);
      mutator_t & mutator = *SharedData().mutator_ptr;
      mutator.ResetLastMutationTracker();
      // Set program constraints
      //mutator.SetProgFunctionCntRange(
      //  {config.PRG_MIN_FUNC_CNT(), config.PRG_MAX_FUNC_CNT()}
      //);
      //mutator.SetProgFunctionInstCntRange(
      //  {config.PRG_MIN_FUNC_INST_CNT(), config.PRG_MAX_FUNC_INST_CNT()}
      //);
      //mutator.SetProgInstArgValueRange(
      //  {config.PRG_INST_MIN_ARG_VAL(), config.PRG_INST_MAX_ARG_VAL()}
      //);
      //const size_t total_inst_limit = 2 * config.PRG_MAX_FUNC_INST_CNT() * config.PRG_MAX_FUNC_CNT();
      //mutator.SetTotalInstLimit(total_inst_limit);
      //mutator.SetFuncNumTags(FUNC_NUM_TAGS);
      //mutator.SetInstNumTags(INST_TAG_CNT);
      //mutator.SetInstNumArgs(INST_ARG_CNT);
      // Set mutation rates
      mutator.SetRateInstArgSub(SharedData().mut_prob);
      mutator.SetRateInstTagBF(0);
      mutator.SetRateInstSub(SharedData().mut_prob);
      mutator.SetRateInstIns(0);
      mutator.SetRateInstDel(0);
      mutator.SetRateSeqSlip(0);
      mutator.SetRateFuncDup(0);
      mutator.SetRateFuncDel(0);
      mutator.SetRateFuncTagBF(0);
      mutator.SetRateInstTagSingleBF(0);
      mutator.SetRateFuncTagSingleBF(0);
      mutator.SetRateInstTagSeqRand(0);
      mutator.SetRateFuncTagSeqRand(0);
    }

  public:
    SignalGPOrg(OrganismManager<SignalGPOrg> & _manager)
      : OrganismTemplate<SignalGPOrg>(_manager), bits(100) { }
    SignalGPOrg(const SignalGPOrg &) = default;
    SignalGPOrg(SignalGPOrg &&) = default;
    SignalGPOrg(const emp::BitVector & in, OrganismManager<SignalGPOrg> & _manager)
      : OrganismTemplate<SignalGPOrg>(_manager), bits(in) { }
    SignalGPOrg(size_t N, OrganismManager<SignalGPOrg> & _manager)
      : OrganismTemplate<SignalGPOrg>(_manager), bits(N) { }
    ~SignalGPOrg() { ; }

    struct ManagerData : public Organism::ManagerData {
      double mut_prob = 0.01;            ///< Probability of each bit mutating on reproduction.
      std::string output_name = "output_val";  ///< Name of output trait (double)
      bool init_random = true;           ///< Should we randomize ancestor?  (false = all zeros)
      emp::Ptr<hardware_t> hardware_ptr;
      emp::Ptr<mutator_t> mutator_ptr;
      inst_lib_t inst_lib;
      event_lib_t event_lib;
      size_t eval_steps = 100;
    };

    /// Use "to_string" to convert.
    std::string ToString() const override { 
      std::stringstream sstr;
      program.Print(sstr, SharedData().inst_lib);
      return sstr.str();
    }

    void GenomeFromString(const std::string & new_genome) override {
      emp_assert(false, "GenomeFromString not implemented");
    }

    size_t Mutate(emp::Random & random) override {
      // Reset mutator's recorded mutations.
      SharedData().mutator_ptr->ResetLastMutationTracker(); 
      const size_t num_muts = SharedData().mutator_ptr->ApplyAll(
        random,
        program
      );
      return num_muts;
    }

    void Randomize(emp::Random & random) override {
      program = sgp::cpu::lfunprg::GenRandLinearFunctionsProgram<hardware_t, 16>(
          random, 
          SharedData().inst_lib
      );
      std::cout << "New genome: \n";
      std::cout << ToString() << std::endl;
    }

    void Initialize(emp::Random & random) override {
      if (SharedData().init_random) Randomize(random);
    }

    /// Put the bits in the correct output position.
    void GenerateOutput() override {
      hardware_t & hardware = *SharedData().hardware_ptr;
      hardware.SetProgram(program);
      hardware.ResetMatchBin();
      hardware.ResetHardwareState();
      hardware.SpawnThreadWithID(0);
      hardware.Process(SharedData().eval_steps);
      
      double val = 0;
      auto& thread = hardware.GetThread(0);
      // Wait, wait. Does this thread have calls on the call stack?
      if (thread.GetExecState().call_stack.size()) {
        auto& call_state = thread.GetExecState().GetTopCallState();
        auto& mem_state = call_state.GetMemory();
        val = mem_state.GetWorking(0);
      }
      SetTrait<double>(SharedData().output_name, val);
    }

    /// Setup this organism type to be able to load from config.
    void SetupConfig() override {
      GetManager().LinkVar(SharedData().eval_steps, "eval_steps",
                      "Number of steps to evaluate");
      GetManager().LinkVar(SharedData().mut_prob, "mut_prob",
                      "Probability of each bit mutating on reproduction.");
      GetManager().LinkVar(SharedData().output_name, "output_name",
                      "Name of variable to contain bit sequence.");
      GetManager().LinkVar(SharedData().init_random, "init_random",
                      "Should we randomize ancestor?  (0 = all zeros)");
    }

    /// Setup this organism type with the traits it need to track.
    void SetupModule() override {
      // Setup the output trait.
      GetManager().AddSharedTrait(SharedData().output_name,
                                  "Output from organism, as a double.",
                                  0.0);
      InitializeHardware();
      InitializeMutator();
    }
  };

  MABE_REGISTER_ORG_TYPE(SignalGPOrg, "Organism based around a SignalGP program that will run on virtual hardware");
}

#endif

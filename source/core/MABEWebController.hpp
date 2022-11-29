/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2022-2022.
 *
 *  @file  MABEWebController.hpp
 *  @brief A derived version of MABE.hpp that provides additional support for building on 
 *    the web 
 */

#ifndef MABE_MABE_WEB_CONTROLLER_HPP
#define MABE_MABE_WEB_CONTROLLER_HPP

// MABE includes
#include "MABE.hpp"
#include "EmptyOrganism.hpp"

// Empirical includes
#include "emp/web/web.hpp"

namespace mabe{
  class MABEWebController : public MABE, emp::web::Animate{
    protected:
    emp::web::Document& doc;   // Div that contains all our web elements
    emp::web::Div bootstrap_container_div; // Div that contains all bootstrap rows
    emp::vector<std::string> row_id_vec;
    size_t steps_per_draw = 1;



    void SetupWebpage(){
      bootstrap_container_div = emp::web::Div("mabe_bootstrap_container");
      bootstrap_container_div.SetAttr("class", "container");
      doc << bootstrap_container_div << "<br/>";

      emp::web::Div controls_row_div("controls_row");
      controls_row_div.SetAttr("class", "row");
      bootstrap_container_div << controls_row_div;

      emp::web::Div button_col_div("control_button_col");
      button_col_div.SetAttr("class", "col-md-12");
      controls_row_div << button_col_div;

      emp::web::Div center_div("", "center");
      button_col_div << center_div;

      emp::web::Button run_button([this](){
          Start();
        }, "Run", "run_button");
      run_button.SetAttr("class", "btn btn-primary");
      center_div << run_button;

      emp::web::Button pause_button([this](){
          Stop();
        }, "Pause", "pause_button");
      pause_button.SetAttr("class", "btn btn-danger");
      center_div << "&nbsp;" << pause_button; 
      
      center_div << "&nbsp" << GetStepButton("anim_step_btn", "Advance Step").SetAttr("class", "btn btn-secondary");
      // Allow user to change the number of attempted cell births per render update
      auto steps_input = emp::web::Input(
          [this](const std::string & s){
            steps_per_draw = std::stoi(s);
          },
          "number", "", "steps_input"
      );
      steps_input.Value(steps_per_draw);
      center_div << "&nbsp;" << steps_input;
      center_div << "<br/>";
    }

    void AddRow(){
      std::stringstream sstr;
      sstr << "mabe_bootstrap_row_" << row_id_vec.size();
      emp::web::Div row_div(sstr.str());
      row_id_vec.push_back(sstr.str());
      row_div.SetAttr("class", "row");
      bootstrap_container_div << row_div;
    }

    std::string AddColumn(size_t row_idx, size_t width){
      emp_assert(row_idx < row_id_vec.size());
      emp::web::Div row_div = doc.Div(row_id_vec[row_idx]);
      size_t col_idx = row_div.Children().size();
      std::stringstream sstr;
      sstr << "mabe_bootstrap_row_" << row_idx + 1 << "_col_" << col_idx + 1;
      const std::string col_div_id = sstr.str();
      emp::web::Div col_div(col_div_id);
      sstr.str("");
      sstr << "col-md-" << width;
      col_div.SetAttr("class", sstr.str());
      col_div << col_div_id << "<br/>";
      row_div << col_div;
      std::cout << "From doc: " << doc.Div(col_div_id).GetID() << std::endl;
      return col_div_id;
    }

    void SetupConfigMethods(){
      GetConfigScript().AddFunction("WEB_MAKE_ROW", 
          [this](){AddRow(); return 0;}, 
          "Add a new bootstrap row to the web page");
      GetConfigScript().AddFunction("WEB_MAKE_COL", 
          [this](size_t row_idx, size_t width){return AddColumn(row_idx, width);}, 
          "Add a new bootstrap row to the web page");
    }

    void DoFrame() override{
      Update(steps_per_draw);
    }
    
    void InitializeModules(){
      for(auto mod_ptr : modules){
        mod_ptr->WebInit();
      }
    }

    public:
    /// Command-line constructor
    MABEWebController(int argc, char* argv[], emp::web::Document& _doc): 
        MABE(argc, argv), 
        doc(_doc)
    {
      SetupEmpty<mabe::EmptyOrganismManager>();
      SetupWebpage();
      SetupConfigMethods();
    }
    MABEWebController(const MABEWebController &) = delete;
    MABEWebController(MABE &&) = delete;
    ~MABEWebController() { ; }
    
    virtual bool IsWeb() override { return true; }

    emp::web::Document& GetDocument(){ return doc; }
    
    void LoadConfig(std::istream& stream){
      Load(stream, "Web input");
      // Setup all of the modules
      if (Setup() == false){
        std::cout << "Error! Setup() failed on MABE object!" << std::endl;
      }
      InitializeModules();
      // Print to the console to make sure MABE object was initialized correctly
      Execute("PRINT(\"Main MABE control object created and setup!\");");
    }
  
  };
}

#endif

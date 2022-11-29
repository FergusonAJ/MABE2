/**
 *  @note This file is part of MABE, https://github.com/mercere99/MABE2
 *  @copyright Copyright (C) Michigan State University, MIT Software license; see doc/LICENSE.md
 *  @date 2019-2020.
 *
 *  @file  MABE.cpp
 *  @brief Dynamic MABE executable that can be fully configured from a file.
 */

#include <iostream>

// Framework
#include "../source/core/MABEWebController.hpp"

// Include the full set of available modules.
#include "../source/modules.hpp"


class WebBase{
protected:
  emp::Ptr<mabe::MABEWebController> web_controller_ptr;// controller(0, NULL);
  emp::web::Document doc;
  emp::web::TextArea config_input;

  void CreateController(){
    std::string input_str = config_input.GetText();
    //if(doc.HasChild("mabe_bootstrap_container")){
    //  doc.ClearChildren();
    //}
    if(web_controller_ptr) web_controller_ptr.Delete();
    web_controller_ptr = 
        emp::NewPtr<mabe::MABEWebController, int, char**, emp::web::Document&>(0, NULL, doc);
    std::stringstream sstr;
    sstr << input_str;
    web_controller_ptr->LoadConfig(sstr);
  }
  
  void SetupWebpage(){
    emp::web::Div bootstrap_div("mabe_config_boostrap_div");
    bootstrap_div.SetAttr("class", "container");
    emp::web::Div row_div("mabe_config_row");
    row_div.SetAttr("class", "row");
    emp::web::Div col_div("mabe_config_col");
    col_div.SetAttr("class", "col-md-12");
    // Card main
    emp::web::Div card_div("mabe_web_card");
    card_div.SetAttr("class", "card");
    card_div.SetCSS("width", "95%");
    // Card body
    emp::web::Div card_body_div("mabe_web_card_body");
    card_body_div.SetAttr("class", "card-title");
    // Card title
    emp::web::Div card_title_div("mabe_web_card_title");
    card_title_div.SetAttr("class", "card-title");
    emp::web::Button load_config_button(
        [this](){ CreateController(); },
        "Load config", "config_load_button");
    load_config_button.SetAttr("class", "btn btn-primary");
    emp::web::Button collapse_button(
        [this](){ ; },
        "Collapse/Expand", "collapse_button");
    collapse_button.SetAttr("class", "btn btn-secondary");
    collapse_button.SetAttr("type", "button");
    collapse_button.SetAttr("data-toggle", "collapse");
    collapse_button.SetAttr("data-target", "#config_input");
    collapse_button.SetAttr("aria-expanded", "false");
    collapse_button.SetAttr("aria-controls", "#config_input");
    emp::web::Div center_div("", "center");
    center_div << "<h2>Config Editor</h2>" <<  load_config_button << "&nbsp;" << collapse_button;
    card_title_div << center_div;
    card_body_div << card_title_div;



    // Setup config input box
    config_input = emp::web::TextArea("config_input");
    config_input.SetCSS("width", "90%");
    config_input.SetCSS("height", "400px");
    card_body_div << "<center>" << config_input << "</center>";
    
    card_div << card_body_div;
    center_div = emp::web::Div("", "center");
    center_div << card_div;
    col_div << center_div;
    row_div << col_div;
    bootstrap_div << row_div;
    doc << bootstrap_div;
  }

public: 

  WebBase() : doc("emp_base"){
    SetupWebpage();
  }
  ~WebBase(){
    if(web_controller_ptr) web_controller_ptr.Delete();
  }
};


WebBase web_base;
int main(){ ; }

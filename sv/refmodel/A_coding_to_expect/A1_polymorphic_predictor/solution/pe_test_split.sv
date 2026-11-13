// pe_test_split.sv : only the override changes; scoreboard code untouched
//   (pe_base_test = your normal test that builds the env with pe_scoreboard)
class pe_test_split extends pe_base_test;
  `uvm_component_utils(pe_test_split)

  function new(string name, uvm_component parent);
    super.new(name, parent);
  endfunction

  function void build_phase(uvm_phase phase);
    // must happen BEFORE super.build_phase builds env -> scoreboard
    set_type_override_by_type(pe_ref_pass::get_type(), pe_ref_split::get_type());
    super.build_phase(phase);
  endfunction
endclass

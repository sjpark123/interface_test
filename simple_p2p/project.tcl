
################################################################################
# Technology Library
################################################################################
set LIB_PATH "[get_install_path]/share/stratus/techlibs/GPDK045/gsclib045_svt_v4.4/gsclib045/timing"
use_tech_lib "$LIB_PATH/slow_vdd1v2_basicCells.lib"

#####################################################################
# c++ compiler options
######################################################################
set CLOCK_PERIOD 5.0 ;# Units is library default time units
set_attr cc_options " -DCLOCK_PERIOD=$CLOCK_PERIOD"     ;# Make clock period available to testbench
set_attr hls_cc_options " -DCLOCK_PERIOD=$CLOCK_PERIOD"

######################################################################
# Recommended project-level Stratus HLS options
# These are a sensible starting point for a good experience and PPA
######################################################################
set_attr message_detail                 1               ;# Detail level in log file: 0, 1, or 2
set_attr rtl_annotation                 op,stack 	;# At a minimum use "id" to allow RTL comments to be added postprocess
set_attr dpopt_auto                     op           	;# Set automatic DPOPT part creation: off, array, op, expr
set_attr undef_func                     error       	;# Error if C++ functions are undefined
set_attr require_pipeline_boundary_wait off		;# Do not insert waits at entry to pipelined loops
set_attr verilog_dialect                systemverilog	;# Verilog format
set_attr wait_for_licenses              all  		;# Enable license waiting ; all|stratus|off
set_attr comm_subexp_elim               on         	;# on|off
set_attr inline_partial_constants       on         	;# on|off
set_attr lsb_trimming                   on         	;# on|off
set_attr shift_trimming                 standard    	;# none|standard|aggressive
set_attr balance_expr                   delay		;# delay|width|off delay is preferred for pipelined designs
set_attr parts_effort                   medium          ;# Medium excludes extra-fast _0 parts
set_attr ignore_scan_cells              on              ;# Don't use scan cells when characterizing parts
set_attr auto_build_hls_libs            on              ;# Automatic update of hls_libs recommended for new designs

# Power
# Remove this if you don't have the XL option
set_attr    power on   ;# enable clock gating, memory power optimization, FSM power optimization

# Treatment of arrays
# Recommended flow is to set flatten_arrays=all, use map_to_memory for specific arrays, and use map_to_reg_bank to improve runtime
set_attr flatten_arrays                 all             ;# Control array flattening: none is the default

######################################################################
# Timing related project-level Stratus HLS options
######################################################################
set_attr clock_period              $CLOCK_PERIOD	        ;# Clock period (required) is period passed to logic synthesis
set_attr cycle_slack               [expr 0.10 * $CLOCK_PERIOD]	;# 10% margin in scheduling clock
set_attr path_delay_limit          111                          ;# sets allocation clock ~= clock_period with 10% scheduling margin

######################################################################
# Testbench or System Level Modules
######################################################################
define_system_module main main.cc
define_system_module system_top system_top.cc
define_system_module tb tb.cc

######################################################################
# SC_MODULEs to be synthesized
######################################################################
set design "dut"
define_hls_module $design ${design}.cc

# Put io_configs here if needed
define_io_config * TLM
define_io_config * PIN

define_hls_config $design BASIC
define_hls_config $design NODPA --dpopt_auto=off

######################################################################
# Genus Logic Synthesis Configurations
######################################################################
foreach config [find -hls_config *] {
  set cname [get_attr name $config]
  define_logic_synthesis_config ${cname}_LS "$design $cname"
}

######################################################################
# Simulation Configurations
######################################################################
# settings for builtin simulator
use_systemc_simulator builtin
use_verilog_simulator xcelium ;# 'xcelium' or 'vcs'
enable_waveform_logging -vcd ;# to store signal transitions vcd, shm (for Xcelium)

# uncomment these lines to use Xcelium simulator for SystemC and Verilog
# settings for Xcelium simulator
# use_systemc_simulator xcelium ;# Uses Xcelium for SystemC and Verilog
# enable_waveform_logging -shm ;# -shm is recommended for Xcelium

# Set script to compare result file to golden file if needed
set_attr end_of_sim_command "make cmp_result"

# A behavioral TLM simulation configuration if needed
define_sim_config T "$design BEH" -io_config TLM

# The following rules are TCL code to create simulation configurations
# for each hls_config defined.
# BEH specifies a behavioral simulation with the -D symbols from the hls_config
# RTL_V specifies a cosimulation with the RTL from the hls_config
foreach config [find -hls_config *] {
  set cname [get_attr name $config]
  define_sim_config ${cname}_B "$design BEH $cname"
  define_sim_config ${cname}_V "$design RTL_V $cname"
}

######################################################################
# Joules Power Estimation Configurations
######################################################################
foreach config [find -hls_config *] {
  set cname [get_attr name $config]
  # Use RTL simulation data and Gate-level design for power analysis
  define_power_config ${cname}_PR ${cname}_V -netlist ${cname}_LS -module $design
}

######################################################################
# Superlint RTL Lint Configurations
######################################################################
foreach config [find -hls_config *] {
  set cname [get_attr name $config]
  define_analysis_config ${cname}_SL "$design $cname" \
    -options  {BDW_ANALYSIS_RULESET BDW_ANALYSIS_CHECK_CODE}
}

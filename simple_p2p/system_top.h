

#ifndef _SYSTEM_TOP_H_
#define _SYSTEM_TOP_H_

// Include some required files.
#include <systemc.h>            // SystemC definitions.
#include <esc.h>                // Cadence ESC functions and utilities.
#include <stratus_hls.h>        // Cadence Stratus definitions.
#include <cynw_p2p.h>           // The cynw_p2p communication channel.
#include "defines.h"
#include "tb.h"
#include "dut_wrap.h"   // use the generated wrapper for all hls_modules

SC_MODULE( system_top )
{
    // clock and reset signals
    sc_clock SC_NAMED(clk_sig, CLOCK_PERIOD, SC_NS );
    sc_signal < bool > SC_NAMED(rst_sig);

    // cynw_p2p channels
    cynw_p2p < DT_1, ioConfig >::chan SC_NAMED(chan1);
    cynw_p2p < DT_2, ioConfig >::chan SC_NAMED(chan2);

    // The testbench and DUT modules.
    dut_wrapper * m_dut;    // use the generated wrapper for all hls_modules
    tb * m_tb;

    SC_CTOR( system_top )
    {
        m_dut = new dut_wrapper( "m_dut" );

        // Connect the design module
        m_dut->clk.bind( clk_sig );
        m_dut->rst.bind( rst_sig );
        m_dut->din.bind( chan1 );
        m_dut->dout.bind( chan2 );

        // Connect the testbench
        m_tb = new tb( "m_tb" );
        m_tb->clk.bind( clk_sig );
        m_tb->rst.bind( rst_sig );
        m_tb->dout.bind( chan1 );
        m_tb->din.bind( chan2 );
    }

    ~system_top()
    {
        delete m_tb;
        delete m_dut;
    }
};

#endif // _SYSTEM_H_


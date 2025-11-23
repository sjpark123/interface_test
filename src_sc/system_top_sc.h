#ifndef _SYSTEM_TOP_SC_H_
#define _SYSTEM_TOP_SC_H_

#include <systemc.h>
#include <cynw_p2p.h>
#include "rgb2ycocg_pixel.h"
// #include "tb.h" // Testbench header (assumed to be created later)

SC_MODULE( system_top )
{
    // Clock and reset signals
    sc_clock SC_NAMED(clk_sig, 10, SC_NS);
    sc_signal<bool> SC_NAMED(rst_sig);

    // cynw_p2p channels
    cynw_p2p< sc_int<16> >::chan SC_NAMED(chan_r);
    cynw_p2p< sc_int<16> >::chan SC_NAMED(chan_g);
    cynw_p2p< sc_int<16> >::chan SC_NAMED(chan_b);
    cynw_p2p< sc_int<16> >::chan SC_NAMED(chan_bits);
    cynw_p2p< ycocg_pixel_t >::chan SC_NAMED(chan_out);

    // The DUT module
    rgb2ycocg_pixel* m_dut;

    // The testbench module (commented out until tb is created)
    // tb* m_tb;

    SC_CTOR( system_top )
    {
        // Instantiate DUT
        m_dut = new rgb2ycocg_pixel("m_dut");

        // Connect DUT
        m_dut->clk(clk_sig);
        m_dut->rst(rst_sig);
        m_dut->din0(chan_r);
        m_dut->din1(chan_g);
        m_dut->din2(chan_b);
        m_dut->din3(chan_bits);
        m_dut->dout(chan_out);

        // Instantiate and connect TB (placeholder)
        /*
        m_tb = new tb("m_tb");
        m_tb->clk(clk_sig);
        m_tb->rst(rst_sig);
        m_tb->dout_r(chan_r);
        m_tb->dout_g(chan_g);
        m_tb->dout_b(chan_b);
        m_tb->dout_bits(chan_bits);
        m_tb->din(chan_out);
        */
    }

    ~system_top()
    {
        delete m_dut;
        // delete m_tb;
    }
};

#endif // _SYSTEM_TOP_SC_H_

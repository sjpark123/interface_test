
#ifndef _DUT_H_
#define _DUT_H_

#include <cynw_p2p.h>
#include "defines.h"

SC_MODULE( dut )
{
    // Declare the clock and reset ports
    sc_in_clk SC_NAMED(clk);
    sc_in < bool > SC_NAMED(rst);

    // Declare the input port and the output port.
    // The template specializations <DT_*> configure the
    // modular interfaces to carry the desired datatypes.
    cynw_p2p < DT_1, ioConfig >::in SC_NAMED(din);    // The input port
    cynw_p2p < DT_2, ioConfig >::out SC_NAMED(dout);  // The output port

    SC_CTOR( dut )
    {
        SC_CTHREAD( thread_function, clk.pos() );
        reset_signal_is( rst, 0 );

        // Give the interfaces access to the clock and reset ports
        din.clk_rst( clk, rst );
        dout.clk_rst( clk, rst );
    }

    void thread_function();                   // the thread function

    DT_2 f( DT_1 );             // the computation function
};

#endif // _DUT_H_

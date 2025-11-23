
#include "dut.h"

// The thread function for the design
void dut::thread_function()
{
    // Reset the interfaces
    // Also initialize any variables that are part of the module class here
    {
        HLS_DEFINE_PROTOCOL( "reset" );
        din.reset();
        dout.reset();

        wait();
    }
    // Main execution loop
    while (1)
    {
        // Get a value, process it, and output the result
        DT_1 in_val = din.get();
        DT_2 out_val;

        out_val = f(in_val);

        dout.put(out_val);
    }
}
//
//  User's computation function
//
DT_2 dut::f(DT_1 var)
{
    return ( var * 7 );
}



#ifndef _TB_H_
#define _TB_H_

#include <queue>
#include <cynw_p2p.h>
#include "defines.h"

SC_MODULE( tb )
{
public:
    // Declare clock and reset ports
    sc_in_clk SC_NAMED(clk);
    sc_out < bool > SC_NAMED(rst);    // The source thread drives rst.

    // Declare modular interface ports
    cynw_p2p < DT_2, ioConfig >::base_in SC_NAMED(din);
    cynw_p2p < DT_1, ioConfig >::base_out SC_NAMED(dout);

    SC_CTOR( tb )
    {
        // Declare the source thread.
        SC_CTHREAD( source, clk.pos() );

        // Declare the sink thread
        SC_CTHREAD( sink, clk.pos() );

        // Give the interfaces access to the clock and reset functions
        din.clk_rst( clk, rst );
        dout.clk_rst( clk, rst );
    }
private:
    void source();
    void sink();

    // Fifo of time data was sent from source thread
    // Used to compute latency and throughput in sink thread
    std::queue<sc_time> time_queue;

    // Compute the clock cycle number for a given time
    int clock_cycle( sc_time );

    ifstream stim_file;         // File stream for reading the stimulus
    ofstream resp_file;         // File stream for writing the responses

    // Utility functions for managing the stimulus file
    void open_stimulus_file( const char * _name = NULL );
    DT_1 read_stimulus_value( bool & eof );
    void close_stimulus_file();

    // Utility functions for managing the response file
    void open_response_file( const char * _name = NULL );
    void write_response_value( DT_2 );
    void close_response_file();

    // Utility function for managing the golden file
    int count_golden_file( const char * _name = NULL );

    // Utility functions for managing performance statistics
    void initialize_statistics();
    void gather_statistics( unsigned long i, DT_2 val );
    void print_statistics();

    // Variables to hold performance statistics
    unsigned long num_samples;
    unsigned long min_latency;
    unsigned long max_latency;
    unsigned long total_latency;

    sc_time start_time;
    sc_time first_time;

};

#endif // _TB_H_



#include "tb.h"
#include <esc.h>                // for the latency logging functions
#include <string>
#include <iostream>

#ifdef DEBUG
#define debug cout
#else
#define debug if(0) cout
#endif

// Source thread
void tb::source()
{
    // Reset the outputs and cycle the design's reset
    dout.reset();
    rst.write( 0 );
    wait( 2 );
    rst.write( 1 );
    wait();

    open_stimulus_file(); // Open the input data file

    // Write a set of values to the dut
    bool eof = false;
    for( unsigned long i = 0; eof == false; i++ )
    {
        DT_1 value = read_stimulus_value( eof );
        if( !eof )
        {
            // output the result
            debug << clock_cycle( sc_time_stamp() ) << " " << name() << " Writing " << value << endl;
            dout.put( value );
            time_queue.push( sc_time_stamp() );
        }
    }

    // If this reaches the timeout it is an error
    wait( 1000 );
    close_stimulus_file();
    cerr << clock_cycle( sc_time_stamp() ) << " " << name() << " Source timing out!" << endl;
    esc_stop();
}

// Read all the expected values from the design
void tb::sink()
{
    din.reset();
    wait();     // to synchronize with reset

    open_response_file(); // Open the output data file

    // read data values until we have read the number that are
    // in the golden file
    int n_samples = count_golden_file();
    for( unsigned long i = 0; i < n_samples; i++ )
    {
        DT_2 inVal = din.get();
        debug << clock_cycle( sc_time_stamp() ) << " " << name() << " Read    " << inVal << endl;

        gather_statistics( i, inVal );

        write_response_value( inVal );  // write to response file
    }

    print_statistics();
    close_response_file();
    esc_stop();
}

int tb::clock_cycle( sc_time time )
{
    // Compute the latency and throughput and record them
    sc_clock * clk_p = dynamic_cast < sc_clock * >( clk.get_interface() );
    sc_time clock_period = clk_p->period(); // get period from the sc_clock object.
    return ( int )( time / clock_period );

}

void tb::open_stimulus_file( const char * _name )
{
    const char * name = ( _name == NULL ? "stimulus.dat" : _name );

    stim_file.open( name );
    if( stim_file.fail() )
    {
        cerr << "Couldn't open " << name << " for reading." << endl;
        exit( 0 );
    }
}

DT_1 tb::read_stimulus_value( bool & eof )
{
    DT_1 value;
    stim_file >> std::ws;       // flush any white space
    if( !stim_file.eof() )
    {
        stim_file >> value;
    }

    eof = ( stim_file.eof() );
    return value;
}

void tb::close_stimulus_file()
{
    stim_file.close();
}

void tb::open_response_file( const char * _name )
{
    const char * name = ( _name == NULL ? "response.dat" : _name );

    // put output file in appropriate directory in bdw_work
    std::string filename = getenv( "BDW_SIM_CONFIG_DIR" );
    filename += "/";
    filename += name;
    resp_file.open( filename.c_str() );
    if( resp_file.fail() )
    {
        cerr << "Couldn't open " << filename << " for writing." << endl;
        exit( 0 );
    }
}

void tb::write_response_value( DT_2 value )
{
    resp_file << value << endl;
}

void tb::close_response_file()
{
    resp_file.close();
}

int tb::count_golden_file( const char * _name )
{
    const char * name = ( _name == NULL ? "golden.dat" : _name );
    ifstream golden_file;         // File stream containing expected values
    unsigned long i;
    DT_2 value;

    golden_file.open( name );
    if( golden_file.fail() )
    {
        cerr << "Couldn't open golden file " << name << "." << endl;
        exit( 0 );
    }

    golden_file >> std::ws;       // flush any white space
    for( i = 0; !golden_file.eof(); i++ )
    {
        golden_file >> value;
        golden_file >> std::ws;       // flush any white space
    }

    golden_file.close();

    return i;
}

// gather per-sample data for later statistical analysis
void tb::gather_statistics( unsigned long i, DT_2 val )
{
    ////
    // gather statistics
    ////
    sc_time sent_time( time_queue.front() );
    time_queue.pop();
    int latency = clock_cycle( sc_time_stamp() - sent_time );
    if( i == 0 )
    {
        num_samples = 0;
        total_latency = 0;
        start_time = sent_time;
        first_time = sc_time_stamp();
        min_latency = latency;
        max_latency = latency;
    }
    if( latency < min_latency )
    {
        min_latency = latency;
    }
    if( latency > max_latency )
    {
        max_latency = latency;
    }
    total_latency += latency;
    num_samples++;
}

// compute and print performance metrics
void tb::print_statistics()
{
    double avg_latency = total_latency / num_samples;

    sc_time last_time = sc_time_stamp();
    double throughput = ( double )( clock_cycle( last_time ) - clock_cycle( first_time ) ) / ( num_samples - 1 );

    // log performance metrics
    esc_log_latency( "dut", ( int )avg_latency, "dut_latency" );
    esc_log_latency( "dut", ( int )throughput, "dut_throughput" );

    // print performance metrics
    cout << "Number of values received     " << num_samples << endl;
    cout << "First value sent at cycle     " << clock_cycle( start_time ) << endl;
    cout << "First value recieved at cycle " << clock_cycle( first_time ) << endl;
    cout << "Last value recieved at cycle  " << clock_cycle( last_time ) << endl;
    cout << "Min latency: " << min_latency << " Max latency: " << max_latency << endl;
    cout << "Average throughput " << throughput << " cycles per value." << endl;
}

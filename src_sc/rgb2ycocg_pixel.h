#ifndef RGB2YCOCG_PIXEL_H
#define RGB2YCOCG_PIXEL_H

#include <cynw_p2p.h>
#include <systemc.h>

// Define ycocg_pixel_t struct for SystemC
struct ycocg_pixel_t {
    sc_int<16> y;
    sc_int<16> co;
    sc_int<16> cg;

    // Default constructor
    ycocg_pixel_t() : y(0), co(0), cg(0) {}

    // Parameterized constructor
    ycocg_pixel_t(sc_int<16> _y, sc_int<16> _co, sc_int<16> _cg) : y(_y), co(_co), cg(_cg) {}

    // Equality operator
    bool operator==(const ycocg_pixel_t& other) const {
        return (y == other.y && co == other.co && cg == other.cg);
    }

    // Assignment operator
    ycocg_pixel_t& operator=(const ycocg_pixel_t& other) {
        y = other.y;
        co = other.co;
        cg = other.cg;
        return *this;
    }
};

// sc_trace overload
inline void sc_trace(sc_trace_file* tf, const ycocg_pixel_t& object, const std::string& name) {
    sc_trace(tf, object.y, name + ".y");
    sc_trace(tf, object.co, name + ".co");
    sc_trace(tf, object.cg, name + ".cg");
}

// Output stream operator overload
inline std::ostream& operator<<(std::ostream& os, const ycocg_pixel_t& obj) {
    os << "(" << obj.y << ", " << obj.co << ", " << obj.cg << ")";
    return os;
}

SC_MODULE(rgb2ycocg_pixel)
{
    // Ports
    sc_in_clk clk;
    sc_in<bool> rst;

    // Input ports (din0: r, din1: g, din2: b, din3: bits)
    cynw_p2p< sc_int<16> >::in din0;
    cynw_p2p< sc_int<16> >::in din1;
    cynw_p2p< sc_int<16> >::in din2;
    cynw_p2p< sc_int<16> >::in din3;

    // Output port
    cynw_p2p< ycocg_pixel_t >::out dout;

    SC_CTOR(rgb2ycocg_pixel)
        : clk("clk"), rst("rst"),
          din0("din0"), din1("din1"), din2("din2"), din3("din3"),
          dout("dout")
    {
        SC_CTHREAD(thread_function, clk.pos());
        reset_signal_is(rst, 0);

        din0.clk_rst(clk, rst);
        din1.clk_rst(clk, rst);
        din2.clk_rst(clk, rst);
        din3.clk_rst(clk, rst);
        dout.clk_rst(clk, rst);
    }

    void thread_function();
    ycocg_pixel_t rgb2ycocg_pixel_func(sc_int<16> r, sc_int<16> g, sc_int<16> b, sc_int<16> bits);
};

#endif // RGB2YCOCG_PIXEL_H

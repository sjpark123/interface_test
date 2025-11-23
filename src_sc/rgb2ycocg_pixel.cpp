#include "rgb2ycocg_pixel.h"

#define REDUCE_CHROMA_16BPC

void rgb2ycocg_pixel::thread_function()
{
    // Reset protocol
    {
        HLS_DEFINE_PROTOCOL("reset");
        din0.reset();
        din1.reset();
        din2.reset();
        din3.reset();
        dout.reset();
        wait();
    }

    while (1)
    {
        // Read inputs
        sc_int<16> r = din0.get();
        sc_int<16> g = din1.get();
        sc_int<16> b = din2.get();
        sc_int<16> bits = din3.get();

        // Compute
        ycocg_pixel_t result = rgb2ycocg_pixel_func(r, g, b, bits);

        // Write output
        dout.put(result);
    }
}

ycocg_pixel_t rgb2ycocg_pixel::rgb2ycocg_pixel_func(sc_int<16> r, sc_int<16> g, sc_int<16> b, sc_int<16> bits)
{
    ycocg_pixel_t result;
    sc_int<17> t; // Use larger bit width for intermediate calculation if needed, or stick to logic
    // Original logic uses int. Let's use sc_int for arithmetic to avoid overflow issues before assignment if necessary,
    // but since inputs are 16-bit, intermediate values might exceed 16 bits.
    // The original code used 'int' which is at least 32-bit.
    // Let's use sc_int<32> for internal calculations to match 'int' behavior.

    sc_int<32> r_int = r;
    sc_int<32> g_int = g;
    sc_int<32> b_int = b;
    sc_int<32> bits_int = bits;
    
    sc_int<32> co_int;
    sc_int<32> cg_int;
    sc_int<32> y_int;
    sc_int<32> t_int;
    sc_int<32> half = 1 << (bits_int - 1);

    // *MODEL NOTE* MN_ENC_CSC
    co_int = r_int - b_int;
    t_int = b_int + (co_int >> 1);
    cg_int = g_int - t_int;
    y_int = t_int + (cg_int >> 1);

#ifdef REDUCE_CHROMA_16BPC
    if (bits_int == 16) {
        co_int = ((co_int + 1) >> 1) + half;
        cg_int = ((cg_int + 1) >> 1) + half;
    } else
#endif
    {
        co_int = co_int + half * 2;
        cg_int = cg_int + half * 2;
    }

    result.y = y_int;
    result.co = co_int;
    result.cg = cg_int;

    return result;
}

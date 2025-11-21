struct mydata {
  mydata() {}
  mydata(sc_uint<8> ain, sc_uint<16> bin) : a(ain), b(bin) {}
  sc_uint<8> a;
  sc_uint<16> b;
  // Comparison operator
  bool operator==(const mydata &other) {
    return ((a == other.a) && (b == other.b));
  }
  // Assignment operator.
  mydata &operator=(const mydata &other) {
    a = other.a;
    b = other.b;
    return *this;
  }
};
inline void sc_trace(sc_trace_file *tf, const mydata &object,
                     const sc_string &name) {
  if (tf) {
    tf->trace(object.a, name + sc_string(".a"));
    tf->trace(object.b, name + sc_string(".b"));
  }
}
inline ostream &operator<<(ostream &os, const mydata &a) {
#ifndef stratus_hls
  os << "(" << a.a.to_string().c_str() << "," << a.b.to_string().c_str()
     << "))";
#endif
  return os;
}

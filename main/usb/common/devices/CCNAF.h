
#ifndef CCNAF_H
#define CCNAF_H

#include "CNAF.h"

class CCNAF {
    private:
        int m_c;
        CNAF m_naf;

    private:
        CCNAF();

    public:
        CCNAF(int c, int n, int a, int f)  : m_c(c), m_naf(n,a,f)
        {}
        CCNAF(int c, const CNAF& naf)  : m_c(c), m_naf(naf)
        {}

        CCNAF(const CCNAF& rhs) : m_c(rhs.m_c),
        m_naf(rhs.m_naf)
        {}


        CCNAF& operator=(const CCNAF& rhs) {
            if (this != & rhs) {
                m_c = rhs.m_c;
                m_naf = rhs.m_naf;
            }
            return *this;
        }

        int c() const { return m_c;}
        int n() const { return m_naf.n();}
        int a() const { return m_naf.a();}
        int f() const { return m_naf.f();}

        CNAF naf() const { return m_naf;}
};


#endif

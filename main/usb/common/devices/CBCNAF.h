
#ifndef CBCNAF_H
#define CBCNAF_H

#include <stdint.h>
#include "CCNAF.h"

class CBCNAF {
    private:
        int m_b; 
        CCNAF m_cnaf;

    public:
        CBCNAF( int b, int c, int n, int a, int f);
        CBCNAF( int b, const CCNAF& cnaf);
        CBCNAF( const CBCNAF& cnaf);
        CBCNAF& operator=( const CBCNAF& cnaf);


        int b() const { return m_b;}
        int c() const { return m_cnaf.c();}
        int n() const { return m_cnaf.n();}
        int a() const { return m_cnaf.a();}
        int f() const { return m_cnaf.f();}

};


#endif

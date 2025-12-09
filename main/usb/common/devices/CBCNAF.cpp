

#include "CBCNAF.h"
#ifndef CBCNAF_CPP
#define CBCNAF_CPP

    CBCNAF::CBCNAF( int b, int c, int n, int a, int f)
: m_b(b), m_cnaf(c,n,a,f)
{}

    CBCNAF::CBCNAF( int b, const CCNAF& cnaf)
: m_b(b), m_cnaf(cnaf)
{}

CBCNAF::CBCNAF( const CBCNAF& rhs)
  : m_b(rhs.m_b), m_cnaf(rhs.m_cnaf)
{}

CBCNAF& CBCNAF::operator=( const CBCNAF& rhs)
{
    if (this!=&rhs) {
        m_b = rhs.m_b;
        m_cnaf = rhs.m_cnaf;
    }
    return *this;
}
#endif

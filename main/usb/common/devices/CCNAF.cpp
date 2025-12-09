#include "CCNAF.h"
#ifndef CCNAF_CPP
#define CCNAF_CPP

CCNAF::CCNAF(int c, int n, int a, int f)
  : m_c(c), m_naf(n,a,f)
{}

CCNAF::CCNAF(int c, const CNAF& naf)
  : m_c(c), m_naf(naf)
{}

CCNAF::CCNAF(const CCNAF& rhs)
: m_c(rhs.m_c),
    m_naf(rhs.m_naf)
{}

CCNAF& CCNAF::operator=(const CCNAF& rhs)
{
    if (this != & rhs) {
        m_c = rhs.m_c;
        m_naf = rhs.m_naf;
    }
    return *this;
}

#endif

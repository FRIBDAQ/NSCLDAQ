
#ifndef CNAF_H
#define CNAF_H

class CNAF {
    private:
        int m_n;
        int m_a;
        int m_f;

    private:
        CNAF();

    public:
        CNAF(int n, int a, int f) : m_n(n), m_a(a), m_f(f)
        {}

        CNAF(const CNAF& rhs) : m_n(rhs.m_n),
            m_a(rhs.m_a),
            m_f(rhs.m_f)
        {}

        CNAF& operator=(const CNAF& rhs) {
            if (this != & rhs) {
                m_n = rhs.m_n;
                m_a = rhs.m_a;
                m_f = rhs.m_f;
            }
            return *this;
        }

        int n() const { return m_n;}
        int a() const { return m_a;}
        int f() const { return m_f;}
};


#endif

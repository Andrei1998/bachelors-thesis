/*
 * Calling the Z3 Theorem Prover on each of the 151 linear programs described in Section 3.3.
 * Requires an installation of the Z3 Theorem Prover and compiling with the -lz3 linker flag.
 */
#include <bits/stdc++.h>
#include <z3++.h>

using namespace std;
using namespace z3;

void check(const int p2[5], const int p3[5], const int c2) {
    // Make variables - r(v, c) will be denoted by get_expr(v, c).
    // A workaround is used to get around the fact that expr is not default
    // constructible (e.g. expressions like r[{2, 3}] would not compile).
    context cx;
    map<pair<int, int>, expr> r;
    auto get_expr = [&r](const int v, const int c) {
        return r.find(make_pair(v, c))->second;
    };
    for (int v = 1; v <= 3; ++v) {
        for (int c = 1; c <= 4; ++c) {
            r.insert(make_pair(make_pair(v, c),
              cx.real_const(("r_" + to_string(v) + "_" + to_string(c)).c_str())));
        }
    }

    // Add constraints.
    solver s(cx);
    // Premise (5).
    s.add(get_expr(1, 1)     == 0);
    s.add(get_expr(2, p2[1]) == 0);
    s.add(get_expr(3, p3[1]) == 0);
    for (int c = 1; c + 1 <= 4; ++c) {
        s.add(get_expr(1, c)     <= get_expr(1, c + 1));
        s.add(get_expr(2, p2[c]) <= get_expr(2, p2[c + 1]));
        s.add(get_expr(3, p3[c]) <= get_expr(3, p3[c + 1]));
    }
    // Condition (2) in Lemma 3.10.
    for (int c = 1; c <= 4; ++c) {
        for (int c1 = 1; c1 <= 4; ++c1) {
            // Or just ">", equivalently, as per Remark 3.11.
            // Both options give "unsat" verdicts, as expected.
            s.add(get_expr(1, c) + get_expr(2, c) + get_expr(2, c1) + get_expr(3, c1)
              >= 1 + get_expr(1, c2) + get_expr(2, c2) + get_expr(3, c2));
        }
    }
    if (s.check() != unsat) {
        cout << s.get_model() << endl;
        exit(1);
    }
}

// Govem a preference profile P = (id, sigma1, sigma2) returns whether P is single-crossing.
bool single_crossing(const int sigma1[5], const int sigma2[5]) {
    int where1[5], where2[5];
    for (int i = 1; i <= 4; ++i) {
        where1[sigma1[i]] = where2[sigma2[i]] = i;
	}
    for (int a = 1; a <= 4; ++a) {
        for (int b = a + 1; b <= 4; ++b) {
            if (where1[a] > where1[b] && where2[a] < where2[b]) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    // List all triples (sigma1, sigma2, c2) such that P = (id, sigma1, sigma2) is single-crossing.
    int sigma1[5] = {0, 1, 2, 3, 4};
    do {
        int sigma2[5] = {0, 1, 2, 3, 4};
        do {
            if (single_crossing(sigma1, sigma2)) {
                static int cnt = 0; ++cnt;
                cerr << "Processing profile " << cnt << endl;  // 151 in total.
                for (int c2 = 1; c2 <= 4; ++c2) {
                    check(sigma1, sigma2, c2);
                }
            }
        } while (next_permutation(sigma2 + 1, sigma2 + 5));
    } while (next_permutation(sigma1 + 1, sigma1 + 5));
    return 0;
}

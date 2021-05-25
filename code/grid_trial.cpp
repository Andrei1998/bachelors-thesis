/*
 * Testing two hypotheses about the structure of optimal k-tilings of grid single-crossing
 * preferences. The main observation used is that:
 *   If a certain property fails for an optimal k-tiling on a given problem instance, then
 *   it also fails on the same instance if all candidates not part of the elected
 *   committee are removed. This means that it is enough to consider the case k = C
 *   and only look at the most preferred candidate of each voter.
 *
 *
 * Hypothesis 1: All optimal k-tilings are sliceable.
 * Results:
 *   Confirmed for:
 *     N <= 8, M <= 8 and C = 4
 *     N <= 4, M <= 5 and C = 5
 *     N <= 3, M <= 6 and C = 5
 *     N <= 3, M <= 3 and C = 6
 *   Additionally confirmed for
 *     N <= 6, M <= 6 and C = 6
 *   under the assumption that neighboring preferences differ
 *   by at most one pair of candidates.
 *
 * Hypothesis 2: All rectangles in an optimal k-tiling touch the sides of the grid.
 * Result: Not true for N = M = 3, C = 5 and the following preference profiles:
 *   01234 02134 03214
 *   12304 21304 32104
 *   41230 42130 43210
 *
 *
 * Notation and technical assumptions:
 *   Voters are pairs of integers from the set {0, ..., N - 1} x {0, ..., M - 1}.
 *   Candidates are integers from the set {0, ..., C - 1}.
 *   Individual preferences are denoted by lists of candidates (e.g. {0, 2, 1} means that 0 > 2 > 1).
 *   Grid preference profiles are denoted by lists of lists of individual preferences (e.g. {{p1, p2},
 *   {p3, p4}} means that voter (0, 0) has preferences p1, ..., voter (1, 1) has preferences p4).
 *   Without loss of generality, voter (0, 0) prefers candidates in order 0 > 1 > ... > C - 1.
 */
#include <bits/stdc++.h>

using namespace std;

// Individual preference list.
using Pref = vector<int>;
// Placeholder for unknown preference lists.
const Pref EmptyProf = vector<int>();

// Given a preference list p, returns the index of candidate c (i.e. 0
// if c is first in the list, 1 if c is second in the list, and so on).
int pos(const Pref& p, const int c) {
    for (int i = 0; i < static_cast<int>(p.size()); ++i) {
        if (p[i] == c) {
            return i;
        }
    }
    throw logic_error("Could not find candidate.");
}
// Given a preference list p, returns whether candidate c0 is prefered over candidate c1.
bool prefers(const Pref& p, const int c0, const int c1) {
    return pos(p, c0) < pos(p, c1);
}
// Given two preference lists p0 and p1, returns the number of unordered pairs of candidates
// (c0, c1) such that c0 is prefered to c1 in p0, but c1 is prefered to c0 in p1, or vice-versa.
int cnt_crosses(const Pref& p0, const Pref& p1, const int C) {
    vector<int> inv_p0(C), inv_p1(C);
    for (int i = 0; i < C; ++i) {
        inv_p0[p0[i]] = inv_p1[p1[i]] = i;
    }
    int ans = 0;
    for (int c0 = 0; c0 < C; ++c0) {
        for (int c1 = c0 + 1; c1 < C; ++c1) {
            ans += ((inv_p0[c0] < inv_p0[c1]) != (inv_p1[c0] < inv_p1[c1]));
        }
    }
    return ans;
}

const int INF = numeric_limits<int>::max();

// Data structure for maintaining bounding boxes. Supports adding points, unioning
// and checking whether the interior intersects a given horizontal/vertical line.
struct Rect {
    int r0, r1;
    int c0, c1;
    Rect(int _r0 = INF, int _r1 = -INF, int _c0 = INF, int _c1 = -INF):
        r0(_r0), r1(_r1), c0(_c0), c1(_c1) {}
    Rect add(const int r, const int c) {
        return Rect(min(r0, r), max(r1, r), min(c0, c), max(c1, c));
    }
    // Returns whether the rectangle intersects the horizontal line between rows r and r + 1.
    bool intersects_with_horizontal(const int r) {
        return r0 <= r && r < r1;
    }
    // Returns whether the rectangle intersects the vertical line between columns c and c + 1.
    bool intersects_with_vertical(const int c) {
        return c0 <= c && c < c1;
    }
};

// Given two bounding boxes r0 and r1, returns whether their intersection is non-empty.
bool do_intersect(const Rect& r0, const Rect& r1) {
    if (r0.r0 > r1.r1) {
        return false;
    } else if (r1.r0 > r0.r1) {
        return false;
    } else if (r0.c0 > r1.c1) {
        return false;
    } else if (r1.c0 > r0.c1) {
        return false;
    } else {
        return true;
    }
}

// Preference profile - a two-dimensional array of preference lists
// (some of which are potentially unknown).
using Grid = vector<vector<Pref>>;

// Prints a preference profile g to stdout.
void show(const Grid& g) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            if (g[i][j] == EmptyProf) {
                cout << "?";
            } else {
                for (int k = 0; k < static_cast<int>(g[i][j].size()); ++k) {
                    cout << g[i][j][k];
                }
            }
            cout << " ";
        }
        cout << endl;
    }
    cout << "####" << endl;
}

// Given a preference profile g and two candidates c0 and c1, returns the
// bounding box of all voters which prefer c0 to c1 in g.
Rect get_preference_bounding_box(const Grid& g, const int c0, const int c1) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);
    Rect ans;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            if (g[i][j] != EmptyProf && prefers(g[i][j], c0, c1)) {
                ans = ans.add(i, j);
            }
        }
    }
    return ans;
}

// Given a preference profile g and a candidate c, returns the bounding
// box of all voters for which c is their most preferred candidate.
Rect get_dominance_box(const Grid& g, const int c) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);
    Rect ans;
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < M; ++j) {
            if (g[i][j][0] == c) {
                ans = ans.add(i, j);
            }
        }
    }
    return ans;
}

// Given a (potentially incomplete) preference profile g, returns false
// if and only if it is certain that it is not single-crossing.
bool grid_valid(const Grid& g, const int C) {
    for (int c0 = 0; c0 < C; ++c0) {
        for (int c1 = c0 + 1; c1 < C; ++c1) {
            if (do_intersect(get_preference_bounding_box(g, c0, c1),
                             get_preference_bounding_box(g, c1, c0))) {
                return false;
            }
        }
    }
    return true;
}

// Given a preference profile g, returns whether all voters have
// the same most preferred candidate.
bool is_monodominated(const Grid& g) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);
    // It is enough to check whether all voters' most preferred candidate is 0
    // since we assumed that voter (0, 0) prefers candidates in order 0 > ... > C - 1.
    Rect r = get_dominance_box(g, 0);
    return r.r0 == 0 && r.c0 == 0 &&
           r.r1 == N - 1 && r.c1 == M - 1;
}

// Given a preference profile g, returns whether the dominance box (as computed by a
// call to "get_dominance_box") of some candidate does NOT touch the four sides of the grid.
bool has_isolated(const Grid& g, const int C) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);
    for (int c = 0; c < C; ++c) {
        Rect r = get_dominance_box(g, c);
        if (r.r0 == INF) {  // c is not the most preferred candidate of any voter.
            continue;
        }
        if (r.r0 > 0 && r.r1 < N - 1 && r.c0 > 0 && r.c1 < M - 1) {
            return true;
        }
    }
    return false;
}

// Given a preference profile g, returns whether there exists a horizontal/vertical
// line which does not intersect the dominance box of any candidate. Note that this
// is the same as the tiling formed by these dominance boxes admitting a split line
// (which is the first condition for a non-trivial sliceable tiling).
bool admits_split_line(const Grid& g, const int C) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);
    for (int i = 0; i + 1 < N; ++i) {
        bool ok = true;
        for (int c = 0; c < C && ok; ++c) {
            Rect r = get_dominance_box(g, c);
            if (r.intersects_with_horizontal(i)) {
                ok = false;
            }
        }
        if (ok) {
            return true;
        }
    }
    for (int j = 0; j + 1 < M; ++j) {
        bool ok = true;
        for (int c = 0; c < C && ok; ++c) {
            Rect r = get_dominance_box(g, c);
            if (r.intersects_with_vertical(j)) {
                ok = false;
            }
        }
        if (ok) {
            return true;
        }
    }
    return false;
}

// Given a (potentially incomplete) preference profile g, returns whether there are two
// voters adjacent in the grid whose preferences differ in more than one pair of candidates.
bool grid_has_fast_cross(const Grid& g, const int C) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);
    for (int i = 0; i + 1 < N; ++i) {
        for (int j = 0; j < M; ++j) {
            if (g[i][j] != EmptyProf && g[i + 1][j] != EmptyProf && cnt_crosses(g[i][j], g[i + 1][j], C) > 1) {
                return true;
            }
        }
    }
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j + 1 < M; ++j) {
            if (g[i][j] != EmptyProf && g[i][j + 1] != EmptyProf && cnt_crosses(g[i][j], g[i][j + 1], C) > 1) {
                return true;
            }
        }
    }
    return false;
}

// Backtracking search - given a (potentially incomplete) grid preference profile g and
// the coordinates of the first voter (r, c) whose preferences have not yet been decided,
// explores the space of complete grid single-crossing profiles which agree with g.
// For each complete single-crossing profile we test our hypotheses.
void backtr(Grid& g, const int C, const int r, const int c) {
    const int N = g.size();
    assert(N > 0);
    const int M = g[0].size();
    assert(M > 0);

    // Experiment modifier: only test grids for which adjacent voters vary in preference
    // by at most one pair of candidates.
    /*if (grid_has_fast_cross(g, C)) {
        return;
    }*/

    // Prune profiles which can not be single-crossing early.
    if (!grid_valid(g, C)) {
        return;
    } else if (r == N) {
        // Monitor progress.
        static int cnt = 0; ++cnt;
        if (cnt % 100 == 0) {
            cerr << "Processed " << cnt << " grid profiles." << endl;
        }
        // Print grids considered.
        //show(g);

        // Hypothesis 1: All optimal k-tilings are sliceable.
        //   N, M, C = 8, 8, 4 OK.
        //   N, M, C = 4, 5, 5 OK.
        //   N, M, C = 3, 6, 5 OK.
        //   N, M, C = 3, 3, 6 OK.
        //   N, M, C = 6, 6, 6 OK (for no "fast crosses").
        if (!admits_split_line(g, C) && !is_monodominated(g)) {
            show(g);
            exit(1);
        }

        // Hypothesis 2: All rectangles in an optimal k-tiling touch the sides of the grid.
        // Does not hold on the following instance:
        //   01234 02134 03214
        //   12304 21304 32104
        //   41230 42130 43210
        /*if (has_isolated(g, C)) {
            show(g);
            exit(1);
        }*/
    } else if (c == M) {
        backtr(g, C, r + 1, 0);
    } else {
        g[r][c].resize(C);
        iota(g[r][c].begin(), g[r][c].end(), 0);
        do {
            backtr(g, C, r, c + 1);
            // The first voter is assumed to always have preferences 0 > ... > C - 1.
            if (r == 0 && c == 0) {
                break;
            }
        } while (next_permutation(g[r][c].begin(), g[r][c].end()));
        g[r][c] = EmptyProf;
    }
}

int main() {
    const int N = 4;
    const int M = 5;
    const int C = 5;
    Grid g(vector<vector<Pref>>(N, vector<Pref>(M, EmptyProf)));
    backtr(g, C, 0, 0);
    return 0;
}

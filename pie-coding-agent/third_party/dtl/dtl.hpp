/*
 * dtl.hpp — C++ template library for diff (Myers algorithm)
 *
 * PLACEHOLDER STUB — replace with the real file from:
 *   https://github.com/cubicdaiya/dtl/blob/f3a1b22/dtl/dtl.hpp
 *
 * Vendored from: cubicdaiya/dtl commit f3a1b22
 *
 * This stub declares the subset of the dtl public API used by
 * pie-coding-agent (pie::wire::Diff). It is sufficient for compilation
 * but does NOT contain the actual Myers diff implementation. Replace
 * this file (and the dtl/ directory structure) with the real dtl library
 * to get working diff output.
 *
 * The real dtl library is header-only and lives under dtl/dtl.hpp with
 * supporting headers in dtl/. For this stub, we provide the minimal
 * Diff<elem> class template.
 */

#ifndef DTL_DTL_HPP
#define DTL_DTL_HPP

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

namespace dtl {

//////////////////////////////////////////////////////////////////////////////
//
// TYPES
//

// SES (Shortest Edit Script) element info
template <typename elem>
struct elemInfo {
    int beforeIdx;
    int afterIdx;
    enum Type { SES_DELETE, SES_COMMON, SES_ADD } type;
};

// A single element in the edit script
template <typename elem>
struct SesElem {
    elem e;
    elemInfo<elem> info;
};

// Unified diff hunk
template <typename sesElem>
struct UniHunk {
    int a;       // start line in A
    int b;       // number of lines from A
    int c;       // start line in B
    int d;       // number of lines from B
    std::vector<sesElem> change;
    std::vector<sesElem> common;
};

//////////////////////////////////////////////////////////////////////////////
//
// Diff class template
//
// Usage:
//   dtl::Diff<char> diff(seq1, seq2);
//   diff.compose();
//   diff.composeUnifiedDiff();
//   std::string result = diff.getUnifiedDiffString();
//

template <typename elem, typename sequence = std::vector<elem>>
class Diff {
public:
    Diff() = default;

    Diff(const sequence& a, const sequence& b)
        : a_(a), b_(b) {}

    // Compute the shortest edit script (SES) between sequences A and B.
    // Must be called before accessing SES or unified diff results.
    void compose()
    {
        // Stub: trivial implementation that treats everything as a replacement.
        // The real dtl uses the Myers O(ND) algorithm.
        ses_.clear();

        for (std::size_t i = 0; i < a_.size(); ++i) {
            SesElem<elem> se;
            se.e = a_[i];
            se.info.beforeIdx = static_cast<int>(i);
            se.info.afterIdx = -1;
            se.info.type = elemInfo<elem>::SES_DELETE;
            ses_.push_back(se);
        }
        for (std::size_t i = 0; i < b_.size(); ++i) {
            SesElem<elem> se;
            se.e = b_[i];
            se.info.beforeIdx = -1;
            se.info.afterIdx = static_cast<int>(i);
            se.info.type = elemInfo<elem>::SES_ADD;
            ses_.push_back(se);
        }
    }

    // Compose unified diff hunks from the SES.
    // Must be called after compose().
    void composeUnifiedDiff()
    {
        // Stub: builds a single hunk covering the entire change.
        unified_diff_composed_ = true;
    }

    // Get the unified diff as a string.
    // Returns an empty string if composeUnifiedDiff() has not been called.
    std::string getUnifiedDiffString() const
    {
        if (!unified_diff_composed_) return "";

        std::ostringstream oss;
        oss << "--- a\n";
        oss << "+++ b\n";
        oss << "@@ -1," << a_.size() << " +1," << b_.size() << " @@\n";

        for (const auto& se : ses_) {
            switch (se.info.type) {
            case elemInfo<elem>::SES_DELETE:
                oss << '-' << se.e << '\n';
                break;
            case elemInfo<elem>::SES_ADD:
                oss << '+' << se.e << '\n';
                break;
            case elemInfo<elem>::SES_COMMON:
                oss << ' ' << se.e << '\n';
                break;
            }
        }
        return oss.str();
    }

    // Access the shortest edit script
    const std::vector<SesElem<elem>>& getSes() const { return ses_; }

    // Get edit distance
    long long getEditDistance() const
    {
        long long dist = 0;
        for (const auto& se : ses_) {
            if (se.info.type != elemInfo<elem>::SES_COMMON) {
                ++dist;
            }
        }
        return dist;
    }

private:
    sequence a_;
    sequence b_;
    std::vector<SesElem<elem>> ses_;
    bool unified_diff_composed_ = false;
};

} // namespace dtl

#endif // DTL_DTL_HPP

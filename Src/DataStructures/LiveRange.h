#ifndef DAPROJECT2_LIVERANGE_H
#define DAPROJECT2_LIVERANGE_H

#include "Point.h"
#include <vector>
#include <set>
#include <string>

/**
 * @brief A single contiguous live range for a given variable.
 *
 * A live range is a sequence of program points along which a specific
 * value of a variable is alive. It must contain exactly one definition
 * point ('+') and exactly one last-use point ('-'), plus any number of
 * intermediate plain points.
 *
 * Example input line:  "i: 1+,2,3,4,5,6-"
 * Produces one LiveRange with points: 1+, 2, 3, 4, 5, 6-
 */

struct LiveRange {
    std::string varName;        ///< Name of the variable this range belongs to
    std::vector<Point> points;  ///< Ordered list of program points in this range

    LiveRange() : varName("") {}
    LiveRange(const std::string& name) : varName(name) {}

    /// Add a point to this live range
    void addPoint(const Point& p) {
        points.push_back(p);
    }

    /// Returns the set of raw line numbers covered by this range
    std::set<int> lineSet() const {
        std::set<int> s;
        for (const auto& p : points)
            s.insert(p.line);
        return s;
    }

    /// Returns true if this range shares at least one line number with another
    bool overlapsWith(const LiveRange& other) const {
        for (const auto& p : points)
            for (const auto& q : other.points)
                if (p.line == q.line)
                    return true;
        return false;
    }

};

#endif //DAPROJECT2_LIVERANGE_H


#ifndef REGISTERALLOC_H
#define REGISTERALLOC_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>

using namespace std;

// Represents a single program point (line number)
// e.g. "7+" -> lineNum=7, isDef=true
//      "10-" -> lineNum=10, isLastUse=true
//      "8"   -> lineNum=8, isDef=false, isLastUse=false
struct ProgramPoint {
    int lineNum;
    bool isDef;     // '+' suffix
    bool isLastUse; // '-' suffix

    ProgramPoint(int n, bool def, bool use)
        : lineNum(n), isDef(def), isLastUse(use) {}

    bool operator<(const ProgramPoint &o) const { return lineNum < o.lineNum; }
    bool operator==(const ProgramPoint &o) const { return lineNum == o.lineNum; }
};

// A live range is an ordered list of program points for one variable
struct LiveRange {
    string varName;
    vector<ProgramPoint> points; // sorted by lineNum

    set<int> lineSet() const {
        set<int> s;
        for (auto &p : points) s.insert(p.lineNum);
        return s;
    }
};

// A web is the union of one or more overlapping/touching live ranges
// of the same variable
struct Web {
    int id;
    string varName;
    vector<ProgramPoint> points; // merged, sorted

    set<int> lineSet() const {
        set<int> s;
        for (auto &p : points) s.insert(p.lineNum);
        return s;
    }

    // Check if this web overlaps (interferes) with another
    // Two webs interfere if they share at least one execution point,
    // EXCEPT when one ends (isLastUse) at the exact point where the other starts (isDef)
    bool interferesWith(const Web &other) const {
        set<int> myLines = this->lineSet();
        set<int> otherLines = other.lineSet();

        for (int line : myLines) {
            if (otherLines.count(line)) {
                // Check the special non-interference case:
                // if 'this' ends here (isLastUse) AND 'other' starts here (isDef) => no interference
                bool thisEndsHere  = false;
                bool otherStartsHere = false;
                for (auto &p : points)
                    if (p.lineNum == line && p.isLastUse) thisEndsHere = true;
                for (auto &p : other.points)
                    if (p.lineNum == line && p.isDef) otherStartsHere = true;

                // symmetric check too
                bool otherEndsHere = false;
                bool thisStartsHere = false;
                for (auto &p : other.points)
                    if (p.lineNum == line && p.isLastUse) otherEndsHere = true;
                for (auto &p : points)
                    if (p.lineNum == line && p.isDef) thisStartsHere = true;

                if ((thisEndsHere && otherStartsHere) || (otherEndsHere && thisStartsHere))
                    continue; // no interference at this point

                return true; // they interfere
            }
        }
        return false;
    }

    // Output format: "web0: 1+,2,3,4,5,6-"
    string toString() const {
        string s = "web" + to_string(id) + ": ";
        for (size_t i = 0; i < points.size(); i++) {
            if (i > 0) s += ",";
            s += to_string(points[i].lineNum);
            if (points[i].isDef)     s += "+";
            if (points[i].isLastUse) s += "-";
        }
        return s;
    }
};

// Config read from the registers file
struct AllocConfig {
    int numRegisters = 0;
    string algorithm = "basic"; // basic | spilling | splitting | free
    int algorithmParam = 0;     // K for spilling/splitting
};

#endif // REGISTERALLOC_H

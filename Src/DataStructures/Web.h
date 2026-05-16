#ifndef DAPROJECT2_WEB_H
#define DAPROJECT2_WEB_H

#include "Point.h"
#include "LiveRange.h"
#include <vector>
#include <map>
#include <string>
#include <set>

/**
 * @brief Represents a "web" — the union of live ranges that overlap or are adjacent.
 *
 * A web is the fundamental unit of register allocation. Instead of mapping
 * variables to registers, the allocator maps webs to registers. Two webs
 * that are simultaneously live cannot share the same register.
 *
 * A web is built by merging LiveRanges of the same variable that share at
 * least one program point (overlap) or are adjacent (one ends with '-' and
 * another starts with '+' on the same line).
 *
 * The points map stores, for each line number, the annotation character
 * ('+', '-', or '\0'). When merging, annotations are preserved:
 *   - '+' and '-' on the same line are both kept (adjacent merge case)
 *   - a plain '\0' is overridden by '+' or '-' if present
 */

struct Web {
    int id;                         ///< Unique web identifier (assigned after all merges)
    std::string varName;            ///< Name of the original variable
    std::map<int, char> points;     ///< line -> annotation ('+', '-', or '\0')
    std::string displayName;        ///< Display name for output, e.g. "web0", "web2.1"
    /**
     * @brief Default constructor, initializes id to -1.
     *
     * @complexity
     * Time: O(1)
     */
    Web() : id(-1) {}
    /**
     * @brief Constructor with id and variable name.
     *
     * @param id Web id.
     * @param name Variable name.
     *
     * @complexity
     * Time: O(1)
     */
    Web(int id, const std::string& name)
        : id(id), varName(name), displayName("web" + std::to_string(id)) {}
    /**
     * @brief Returns web name.
     *
     * @return Web's name.
     *
     * @complexity
     * Time: O(1)
     */
    std::string webName() const {
        return displayName.empty() ? "web" + std::to_string(id) : displayName;
    }

    /**
     * @brief Merges a LiveRange into this web.
     *
     * Adds all its points, preserving annotations.
     * If a line already exists, a '+' or '-' takes priority over '\0'.
     *
     * @param range The LiveRange to merge into the web
     *
     * @complexity
     * Time: O(N log M), where N is the number of points in the LiveRange
     * and M is the number of points already in the Web
     */
    void merge(const LiveRange& range) {
        for (const auto& p : range.points) {
            auto it = points.find(p.line);
            if (it == points.end()) {
                points[p.line] = p.annotation;
            } else {
                // Keep '+' or '-' over plain '\0'
                if (it->second == '\0' && p.annotation != '\0')
                    it->second = p.annotation;
            }
        }
    }
    /**
     * @brief Returns the set of line numbers covered by this web.
     *
     * @return A set containing all line numbers in the web
     *
     * @complexity
     * Time: O(M log M), where M is the number of points in the web
     */
    /// Returns the set of raw line numbers covered by this web
    std::set<int> lineSet() const {
        std::set<int> s;
        for (const auto& [line, _] : points)
            s.insert(line);
        return s;
    }

    /**
     * @brief Checks if this web interferes with another web.
     *
     * Two webs interfere if there is at least one program point where both
     * are simultaneously live — EXCEPT when one web starts with '+' (def)
     * and the other ends with '-' (last use) on the exact same line.
     * In that case they do NOT interfere (use happens before def at that point).
     *
     * @param other The other web to check against
     * @return true if the two webs interfere
     *
     * @complexity
     * Time: O(M log N), where N is the number of points in the LiveRange
     * and M is the number of points already in the Web
     */
    bool interferesWith(const Web& other) const {
        for (const auto& [lineA, annotA] : points) {
            auto it = other.points.find(lineA);
            if (it == other.points.end()) continue;

            char annotB = it->second;

            // They share line lineA — check the non-interference exception:
            // A starts (def '+') and B ends (last use '-') -> no interference
            // B starts (def '+') and A ends (last use '-') -> no interference
            bool aDefBUse = (annotA == '+' && annotB == '-');
            bool bDefAUse = (annotB == '+' && annotA == '-');

            if (!aDefBUse && !bDefAUse)
                return true;
        }
        return false;
    }

    /**
     * @brief Produces the formatted output string for this web's points.
     *
     * Points are listed in ascending order with their annotations.
     * Example: "1+,2,3,4,5,6-"
     *
     * @return Formatted string representing the points in the web.
     *
     * @complexity
     * Time: O(M), where M is the number of points in the web.
     */
    std::string pointsString() const {
        std::string result;
        bool first = true;
        for (const auto& [line, annot] : points) {
            if (!first) result += ',';
            result += std::to_string(line);
            if (annot != '\0') result += annot;
            first = false;
        }
        return result;
    }
};

#endif //DAPROJECT2_WEB_H

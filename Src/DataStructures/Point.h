#ifndef DAPROJECT2_POINT_H
#define DAPROJECT2_POINT_H

/**
 * @brief Represents a single annotated program point in a live range.
 *
 * A program point is a line number with an optional annotation:
 *   '+' -> the variable is defined (written) at this line (LHS of assignment)
 *   '-' -> the variable is last used (read) at this line (RHS, end of range)
 *   '\0' -> the variable is simply live at this line (no def or last-use)
 *
 * Example: parsing "7+,8,9,10-" yields:
 *   { 7, '+' }, { 8, '\0' }, { 9, '\0' }, { 10, '-' }
 */

struct Point {
    int line;           ///< Program line number (1-based)
    char annotation;    ///< '+', '-', or '\0'
    /**
     * @brief Creates a Point with a line and annotation.
     *
     * @param line Program line number.
     * @param annotation Annotation character ('+', '-', or '\0').
     *
     * @complexity
     * Time: O(1)
     */
    Point(int line, char annotation = '\0')
        : line(line), annotation(annotation) {}
    /**
     * @brief Checks if this point is a definition (+).
     *
     * @return true if annotation is '+'
     *
     * @complexity
     * Time: O(1)
     */
    bool isDef()     const { return annotation == '+'; }
    /**
     * @brief Checks if this point is the last use of a variable (-).
     *
     * @return true if annotation is '-'.
     *
     * @complexity
     * Time: O(1)
     */
    bool isLastUse() const { return annotation == '-'; }
    /**
     * @brief Checks if this point is plain (no annotation).
     *
     * @return true if annotation is '\0'.
     *
     * @complexity
     * Time: O(1)
     */
    bool isPlain()   const { return annotation == '\0'; }
    /**
     * @brief Comparison operator for ordering Points by line number.
     *
     * @param other Another point to compare against.
     * @return true if this line number is less than the other
     *
     * @complexity
     * Time: O(1)
     */
    bool operator<(const Point& other) const { return line < other.line; }
    /**
     * @brief Equality operator for Points.
     *
     * @param other Another point to compare against.
     * @return true if both points have the same line number.
     *
     * @complexity
     * Time: O(1)
     */
    bool operator==(const Point& other) const { return line == other.line; }
};

#endif //DAPROJECT2_POINT_H

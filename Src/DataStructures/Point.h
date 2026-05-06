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

    Point(int line, char annotation = '\0')
        : line(line), annotation(annotation) {}

    bool isDef()     const { return annotation == '+'; }
    bool isLastUse() const { return annotation == '-'; }
    bool isPlain()   const { return annotation == '\0'; }

    bool operator<(const Point& other) const { return line < other.line; }
    bool operator==(const Point& other) const { return line == other.line; }
};

#endif //DAPROJECT2_POINT_H

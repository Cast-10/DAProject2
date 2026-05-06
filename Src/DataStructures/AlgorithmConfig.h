#ifndef DAPROJECT2_ALGORITHMCONFIG_H
#define DAPROJECT2_ALGORITHMCONFIG_H

#include <string>

/**
 * @brief Algorithm variant identifiers.
 */
enum class AlgorithmType {
    BASIC,      ///< "algorithm: basic"           — greedy coloring, no fallback
    SPILLING,   ///< "algorithm: spilling, K"     — spill up to K webs to memory
    SPLITTING,  ///< "algorithm: splitting, K"    — split up to K webs
    FREE        ///< "algorithm: free"             — custom approach
};

/**
 * @brief Configuration parsed from the registers input file.
 *
 * Example input file:
 *   registers: 3
 *   algorithm: spilling, 2
 *
 * Produces:  numRegisters = 3, type = SPILLING, param = 2
 */
struct AlgorithmConfig {
    int numRegisters;       ///< Maximum number of physical registers available
    AlgorithmType type;     ///< Which algorithm variant to run
    int param;              ///< Numeric parameter for spilling/splitting (K), -1 if unused

    AlgorithmConfig()
        : numRegisters(0), type(AlgorithmType::BASIC), param(-1) {}

    /// Human-readable name of the algorithm type
    std::string typeName() const {
        switch (type) {
            case AlgorithmType::BASIC: return "basic";
            case AlgorithmType::SPILLING: return "spilling";
            case AlgorithmType::SPLITTING: return "splitting";
            case AlgorithmType::FREE: return "free";
        }
        return "unknown";
    }
};

#endif //DAPROJECT2_ALGORITHMCONFIG_H

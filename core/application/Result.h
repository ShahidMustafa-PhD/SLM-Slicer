#ifndef RESULT_H
#define RESULT_H

#include <string>

namespace MarcSLM {
namespace Application {

/**
 * @brief Result type for use case operations
 * 
 * Provides a clean way to return success/failure from use cases
 * with optional error messages, avoiding exceptions for control flow.
 */
class Result {
public:
    static Result success() {
        return Result(true, "");
    }
    
    static Result error(const std::string& message) {
        return Result(false, message);
    }
    
    bool isSuccess() const { return m_success; }
    bool isError() const { return !m_success; }
    std::string errorMessage() const { return m_errorMessage; }
    
private:
    Result(bool success, const std::string& message)
        : m_success(success), m_errorMessage(message) {}
    
    bool m_success;
    std::string m_errorMessage;
};

} // namespace Application
} // namespace MarcSLM

#endif // RESULT_H

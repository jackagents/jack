#ifndef JACK_BELIEFQUERY_H
#define JACK_BELIEFQUERY_H

#include <functional>   // for function
#include <type_traits>  // for move
#include <vector>       // for vector
#include <string>       // for string, basic_string

namespace aos { namespace jack {

class BeliefContext;

struct ASTNode
{
    enum State
    {
        SYMBOL,
        BINOP,
        METHOD,
        NUMBER,
        UNIOP
    };

    ASTNode(State state, const std::string &text);
    void addChild(ASTNode &&child);

    State state;
    std::string symbol;
    std::vector<ASTNode> children;
};

/*! ***********************************************************************************************
 * \class   BeliefQuery
 *
 * JACK's belief query language wrapper
 * ************************************************************************************************/
class BeliefQuery
{
public:
    BeliefQuery();

    BeliefQuery(const std::function<bool(const BeliefContext&)> func);

    BeliefQuery(const std::string &query);

    BeliefQuery(const BeliefQuery &other);

    BeliefQuery& operator=(const BeliefQuery &other);

    bool isValid() const { return m_valid; }

    bool isLambda() const { return m_condition ? true : false; }

    bool isQuery() const { return !m_query.empty(); }

    /// Execute this queries lambda
    bool executeLambda(const BeliefContext &context) const;

    /// Execute the query directly by passing in the agent
    bool operator()(const BeliefContext& context) const;

    /// @return A list of beliefs we are interested in
    const std::vector<std::string> &getSubscribedBeliefs() const { return m_symbols; }

    /// Set a list of beliefs we are interested in
    void setSubscribedBeliefs(const std::vector<std::string> &symbols) { m_symbols = symbols; }

    /// Set a callback for when this query becomes true
    void onTrue(const std::function<void()> func) { m_onTrue = func; }

private:

    /// \todo Better documentation
    bool executeAST(const BeliefContext& context) const;

    /// Construct an AST from the query string
    void parse();

    /// True if a condition of query was provided at time of construction
    bool m_valid;

    /// The lambda executed when executeLambda is called
    std::function<bool(const BeliefContext &)> m_condition;

    /// \todo Better documentation
    std::string m_query;

    /// Execution command buffer - executed as a stack in lisp style
    std::vector<ASTNode> m_stack;

    /// \todo Better documentation
    std::vector<std::string> m_symbols;

    /// \todo Better documentation
    std::function<void()> m_onTrue;
};

}} // namespace aos::jack

#endif

#include <jack/beliefquery.h>
#include <jack/beliefcontext.h>  // for BeliefContext
#include <jack/bqltokens.h>      // for BQL_CLOSE_PARENTHESES, BQL_COMMA, BQL_EQUAL
#include <jack/corelib.h>

#include <map>              // for map
#include <ostream>          // for operator<<, basic_ostream
#include <string>           // for char_traits, stoi

extern "C" {
    void scan_string(const char *str);
    char *yyget_text(void);
    int yylex(void);
    int yywrap(void)
    {
        return 1;
    }
    void delete_current_buffer();
}

namespace aos { namespace jack {

ASTNode::ASTNode(State state, const std::string &text)
: state(state)
, symbol(text)
{

}

void ASTNode::addChild(ASTNode &&child)
{
    children.push_back(std::move(child));
}

BeliefQuery::BeliefQuery()
    : m_valid(false)
{
}

BeliefQuery::BeliefQuery(const std::function<bool(const BeliefContext&)> func)
    : m_valid(true)
    , m_condition(func)
{
}

BeliefQuery::BeliefQuery(const std::string &query)
    : m_valid(true)
    , m_query(query)
{
    // TODO: now we do this straight away but
    // eventually we would do this in a lazy fashion.
    // i.e. when required.
    parse();
}

BeliefQuery::BeliefQuery(const BeliefQuery &other)
    : m_valid(other.m_valid)
    , m_condition(other.m_condition)
    , m_query(other.m_query)
    , m_stack(other.m_stack)
{
}

BeliefQuery& BeliefQuery::operator=(const BeliefQuery &other)
{
    if (other.m_valid) {
        m_valid = other.m_valid;
        m_condition = other.m_condition;
        m_query = other.m_query;
        m_stack = other.m_stack;
    }

    return *this;
}

bool BeliefQuery::executeLambda(const BeliefContext &context) const
{
    bool result = m_condition ? m_condition(context) : false;
    if (result == true && m_onTrue) {
        m_onTrue();
    }
    return result;
}

// execute the query directly by passing in the agent
bool BeliefQuery::operator()(const BeliefContext& context) const {
    if (isLambda()) {
        return executeLambda(context);
    }
    else if (!m_query.empty())
    {
        // this method really needs to be const
        // so the AST is really a caching mechanism
        // so we can make it mutable
        /*if (m_stack.empty()) {
            // if AST is not parsed  - do it now ( lazy evaluation )
            parse();
        }*/

        return executeAST(context);
    }

    return false;
}

bool BeliefQuery::executeAST(const BeliefContext &bs) const
{
    /*! ***************************************************************************************
     * Execute this belief's query AST stack
     *
     * 1.
     * 2.
     * ****************************************************************************************/

    bool result = false;

    // only execute a single bin op for now
    for(auto& s : m_stack)
    {
        // perform binary truth
        if (s.state == ASTNode::SYMBOL) {

            JACK_DEBUG("evaluating boolean symbol {}", s.symbol);

            bool r = false;
            if (bs.get<bool>(s.symbol, r))
            {
                if (r) {
                    JACK_DEBUG_MSG("symbol is true");
                }
                else {
                    JACK_DEBUG_MSG("symbol is false");
                }

                result = r;
            }
            // else // how do we coerce custom belief types?
        }
        // perform a uni operator
        else if (s.state == ASTNode::UNIOP) {
            // only handles ! at the moment
            if (s.children.size() >= 1)
            {
                if (s.children[0].state == ASTNode::SYMBOL) {
                    bool v;
                    if (!bs.get<bool>(s.children[0].symbol, v)) {
                        JACK_WARNING("symbol {} not found in belief set", s.children[0].symbol);
                    }

                    if (s.symbol == "!") {
                        result =  !v;
                    }
                }
            }
        }
        // perform binop
        else if(s.state == ASTNode::BINOP) {

            JACK_DEBUG("executing binop({}) num children({})", s.symbol, s.children.size());

            if (s.children.size() >= 2)
            {
 //               std::cout << "executing binop " << s.symbol << std::endl;

                // handle int
                int left = 0;

                if (s.children[0].state == ASTNode::NUMBER)
                {
                    left = std::stoi(s.children[0].symbol);
                }
                else if (s.children[0].state == ASTNode::SYMBOL)
                {
                    if (!bs.get<int>(s.children[0].symbol, left)) {
                        JACK_WARNING("symbol {} not found in belief set", s.children[0].symbol);
                    }
                }

                int right = 0;

                if (s.children[1].state == ASTNode::NUMBER)
                {
                    right = std::stoi(s.children[1].symbol);
                }
                else if (s.children[1].state == ASTNode::SYMBOL)
                {
                    if (!bs.get<int>(s.children[1].symbol, right)) {
                        JACK_WARNING("symbol {} not found in belief set", s.children[1].symbol);
                    }
                }

                JACK_DEBUG("executing binop {}{}{}", left, s.symbol, right);

                if (s.symbol == "==") {
                    result =  left == right;
                } else if (s.symbol == "<") {
                    result = left < right;
                } else if (s.symbol == ">") {
                    result = left > right;
                }

 //               std::cout << left << " < " << right << ((left < right) ? " = true" : " = false") << std::endl;
            }
            //for(auto& s2 : s.children) {
            //   std::cout << s2.symbol << std::endl;
            //}
        }

        //std::cout << s.symbol << std::endl;

        //for(auto& s2 : s.children)
        //{
        //    std::cout << s2.symbol << std::endl;
        //}
    }

    if (result == true && m_onTrue) {
        m_onTrue();
    }

    return result;
}

// construct a AST from the query string
void BeliefQuery::parse()
{
    // scan the query into flex
    scan_string(m_query.c_str());

    int ntoken = yylex();

    while(ntoken > 0) {
        switch(ntoken) {
            case BQL_SYMBOL:
            {
                JACK_DEBUG("Symbol {}", yyget_text());

                if (!m_stack.empty()) {
                    ASTNode& last = m_stack.back();
                    if (last.state == ASTNode::BINOP || last.state == ASTNode::UNIOP) {
                        last.addChild(ASTNode(ASTNode::SYMBOL, yyget_text()));
                    } else {
                        m_stack.push_back(ASTNode(ASTNode::SYMBOL, yyget_text()));
                    }
                } else {
                    m_stack.push_back(ASTNode(ASTNode::SYMBOL, yyget_text()));
                }

                // this symbol is of interest so we store it in a seperate list
                // we will need to check that this belief exists in the belief set
                // and use these to subscribe to changes in the belief sets
                m_symbols.push_back(yyget_text());
                break;
            }
            case BQL_OPEN_PARENTHESES:
            {
                JACK_DEBUG_MSG("Open Parentheses");
                ASTNode left1 = m_stack.back();
                if (left1.state == ASTNode::SYMBOL) {
                    m_stack.pop_back();
                    ASTNode node(ASTNode::METHOD, left1.symbol);
                    m_stack.push_back(node);
                }
                break;
            }
            case BQL_CLOSE_PARENTHESES:
                JACK_DEBUG_MSG("Close Parentheses");
                break;
            case BQL_LESS_THAN_OP:
            {
                JACK_DEBUG_MSG("< Op");
                // pop sym and push op
                ASTNode left2 = m_stack.back();
                if (left2.state == ASTNode::SYMBOL) {
                    m_stack.pop_back();
                    ASTNode node(ASTNode::BINOP, std::string("<"));
                    node.addChild(std::move(left2));
                    m_stack.push_back(node);
                }
                break;
            }
            case BQL_GREATER_THAN_OP:
            {
                JACK_DEBUG_MSG("> Op");
                // pop sym and push op
                ASTNode left2 = m_stack.back();
                if (left2.state == ASTNode::SYMBOL) {
                    m_stack.pop_back();
                    ASTNode node(ASTNode::BINOP, std::string(">"));
                    node.addChild(std::move(left2));
                    m_stack.push_back(node);
                }
                break;
            }
            case BQL_EQUAL:
            {
                JACK_DEBUG_MSG("== Op");
                // pop sym and push op
                ASTNode left2 = m_stack.back();
                if (left2.state == ASTNode::SYMBOL) {
                    JACK_DEBUG("adding == bin op. left is {}", left2.symbol);
                    m_stack.pop_back();
                    ASTNode node(ASTNode::BINOP, std::string("=="));
                    node.addChild(std::move(left2));
                    m_stack.push_back(node);
                }
                else
                {
                    JACK_WARNING_MSG("left child is not a symbol");
                }

                break;
            }
            case BQL_COMMA:
                JACK_DEBUG_MSG("Comma");
                break;
            case BQL_NUMBER:
            {
                JACK_DEBUG("Number {}", yyget_text());

                ASTNode& last = m_stack.back();
                if (last.state == ASTNode::BINOP || last.state == ASTNode::UNIOP) {

  //                  std::cout << "completing binop " << m_stack[0].children[0].symbol << std::endl;

                    //ASTNode node(ASTNode::ASTNode::NUMBER, std::atoi(yyget_text()));
                    ASTNode node(ASTNode::NUMBER, yyget_text());
                    last.addChild(std::move(node));

  //                  std::cout << "completing binop " << m_stack[0].children.size() << std::endl;
                }

                // what do we do with this?
                // push into previous node unless it's complete
                break;
            }
            case BQL_EXCLAMATION:
            {
                JACK_DEBUG("UniaryOp(){}", yyget_text());
                ASTNode node(ASTNode::UNIOP, std::string("!"));
                m_stack.push_back(node);

                break;
            }
        }

        ntoken = yylex();
            //std::cout << yylex() << std::endl;
    }

    // print out the list of symbols to subscibe to
    for (auto s : m_symbols)
    {
        JACK_DEBUG("symbol parsed {}", s);
    }

    // clean up flex
    delete_current_buffer();
}

}} // namespace aos::jack

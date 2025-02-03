// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

using namespace aos;

#if 0
#define PROP(T, V, G, S)      \
    void S(const T &v)        \
    {                         \
        V = v;                \
        S##_impl();           \
    }                         \
    T G() const { return V; } \
    T V;                      \
    virtual void S##_impl() { set<T>(#V, V); }

class SimpleModel : public jack::CustomBeliefSet<SimpleModel>
{
public:

    SimpleModel(const std::string &name)
        : jack::CustomBeliefSet<SimpleModel>(name)
    {
        setCount(0);
        setFlag(false);
    }

    PROP(int, count, getCount, setCount)
    PROP(bool, flag, getFlag, setFlag)
};

TEST(BeliefSetTests, TestCustom)
{
    bool done = false;
    bool zerodone = false;

    jack::BeliefContext context;
    SimpleModel *bs = new SimpleModel("ABC");

    context.addBeliefSet(bs);

    int c;

    EXPECT_EQ( bs->get<int>("count", c), true);

    jack::BeliefQuery bqzero("count == 0");

    bqzero.onTrue([&]() {
        std::cout << "count is 0" << std::endl;
        zerodone = true;
    });

    bs->subscribe(bqzero); // should this trigger immediately?

    bs->refreshSubscribers("count"); // refresh any query using count

    EXPECT_EQ(zerodone, true);

    jack::BeliefQuery bq("count == 5");

    bq.onTrue([&]() {
        std::cout << "count is now 5" << std::endl;
        done = true;
    });

    bs->subscribe(bq);

    // it should trigger if we set the count to 5
    bs->setCount(5);
    EXPECT_EQ(done, true);

    // reset the check
    done = false;

    // it should not trigger if we set the count to 4
    bs->setCount(4);
    EXPECT_EQ(done, false);

    // if should re-trigger if we set the count to 5
    bs->setCount(5);
    EXPECT_EQ(done, true);
}
#endif

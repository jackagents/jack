// © LUCAS FIELD AUTONOMOUS AGRICULTURE PTY LTD, ACN 607 923 133, 2025

#include <jack/jack.h>
#include <gtest/gtest.h>

using namespace aos;

/*! ***********************************************************************************************
 * \class   ResourcesTest
 *
 * This google test fixture aims to provide a context for Resources specific testing
 * See "3.1.2 Resources" section of Use Cases & Requirements
 * ************************************************************************************************/
class ResourcesTest : public ::testing::Test {
    protected:
    ResourcesTest()
    {
    }

    void SetUp() override
    {

    }

    jack::Engine bdi;
};

//TODO: provide operator overrides
class Data {

public:
    std::string name;
    int         value;
    bool        flag;
};

bool operator!=(Data d1, Data d2)
{
    return  d1.name != d2.name ||
            d1.value != d2.value ||
            d1.flag != d2.flag;
}

bool operator==(Data d1, Data d2)
{
    return  d1.name == d2.name &&
            d1.value == d2.value &&
            d1.flag == d2.flag;
}

TEST_F(ResourcesTest, F11_ExternalDataSupport)
{
    /*! ***************************************************************************************
     * Verify external stuff can be used within JACK code
     *
     * F11
     * Support to external native interfaces shall be provided
     * Custom data model, API, I/O interfaces, etc…
     * Ref. S01 V04 V06 V21 P01 P13
     * ****************************************************************************************/
    Data data1;
    Data data2;

    jack::BeliefSet bs;
    //bs.set<Data>("data", data1);

    bdi.action("Action").commit();

    auto plan = bdi.plan ("Plan")
            .handles    ("Goal")
            .pre        ([&] (const jack::BeliefContext&) {
                            return true;
                        })
            .body       (bdi.coroutine ()
                            .print("Plan body\n")
                            //.action("Action")
                         )
            .commit     ();

    //jack::BeliefSetBuilder<Data> bs = bdi.beliefset<Data>();

    jack::Agent* agent = bdi.agent("Agent")
            .plans  ({plan})
            //.beliefs(bs)
            .handleAction   ("Action", [&] (jack::Agent& agent, jack::Action &action) {
                                std::cout << "Goal! " << std::endl;
                                return jack::Action::SUCCESS;
                            })
            .create ("Agent1");

    bdi.goal("Goal").commit ();

    agent->pursue ("Goal");
    agent->start ();

    bdi.poll (1000);
}

#if 0 // removed until implemented
TEST_F(ResourcesTest, F12_ConcurrentDataAccess)
{
    /*! ***************************************************************************************
     * Verify data can be safely accessed by different threads
     *
     * F12
     * Concurrent access to shared data such as beliefset shall be regulated
     * Beliefsets may be accessed from other threads for reading or writing or both.
     * Ref. S02 S03 C03 C23 V04 V12 V22
     * ****************************************************************************************/
    EXPECT_TRUE (false);
}


TEST_F(ResourcesTest, F13_BeliefsetTriggers)
{
    /*! ***************************************************************************************
     * Verify appropriate beliefset trigger behaviour
     *
     * F13
     * Events shall be triggered automatically when beliefs are added, deleted or changed
     * onAdded, onRemoved, onChanged
     * Ref. C01 C03 C11 C34
     * ****************************************************************************************/
    EXPECT_TRUE (false);
}

TEST_F(ResourcesTest, F14_Timestamps)
{
    /*! ***************************************************************************************
     * Verify the beleifsets can natitvely keep track of time (what time?)
     *
     * F14
     * Beliefset entries shall provide supports to timestamps
     * Expiration time or most recent entry, etc…
     * Ref. C24
     * ****************************************************************************************/
    EXPECT_TRUE (false);
}


TEST_F(ResourcesTest, F15_ObservableTrigger)
{
    /*! ***************************************************************************************
     * Verify external stuff can be used within JACK code
     *
     * F15
     * Events shall be triggered when observed objects are changed
     * Custom data models and external entities shall be observable
     * Ref. S01 C10 V21
     * ****************************************************************************************/
    EXPECT_TRUE (false);
}

#endif

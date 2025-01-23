#include <jack/dag.h>
#include <jack/plan.h>      // for Plan
#include <jack/schedule.h>  // for SearchNode, Schedule

/// Third Party
#include <tracy/Tracy.hpp>
#include <cassert>     // for assert
#include <string>      // for string, operator==

namespace aos { namespace jack {

/* ************************************************************************************************
 * Public Ctor & Dtor
 * ************************************************************************************************/

static bool hasResourceDependency(DAGNode *a, DAGNode *b)
{
    const Plan *aPlan = a->node->plan;
    const std::vector<std::string> &aResources = aPlan->resourceLocks();

    const Plan *bPlan = b->node->plan;
    const std::vector<std::string> &bResources = bPlan->resourceLocks();

    bool result = false;
    for (const std::string &aResource : aResources) {
        for (const std::string &bResource : bResources) {
            if (aResource == bResource) {
                result = true;
                break;
            }
        }
    }

    return result;
}

void IntentionExecutionDAG::setSchedule(Schedule *schedule)
{
    ZoneScoped;
    m_open.clear();
    m_dagNodes.clear();

    if (!schedule) {
        return;
    }

    /// Get the best schedule
    std::vector<SearchNode*> bestIntentions = schedule->getBestIntentions();
    if (bestIntentions.empty()) {
        return;
    }

    m_dagNodes.reserve(bestIntentions.size());
#if 0
    // just add all the nodes for now - no dependency
    for(auto node : nodes) {
        DAGNode *dagNode = JACK_NEW(DAGNode);
        dagNode->node = node;
        dagNode->dependencyCounter = 0;
        m_open.push_back(dagNode);
        m_openCount++;
    }
#else

    // Given the best chain of search nodes to execute, convert the linear execution list of nodes
    // (SearchNodes) into a series of execution lists consisting of node's (DAGNodes) that can
    // be executed in parallel as IntentionExecutor's whilst ensuring any plans with resource
    // conflicts are sequenced as necessary.
    //
    // i.e. Given this chain of best search nodes
    //
    //     [Intention 00] --> [Intention 01] --> [Intention 02] --> [Intention 03]
    //     [Locks(A)    ]     [Locks(B)    ]     [Locks(A, B) ] --> [Locks(B)    ]
    //
    // Convert the linear chain to the following 2 lists ensuring any resource dependencies
    // [represented by Lock(X)] are queued up under the appropriate list.
    //
    //     [Intention 00] --> [Intention 02]
    //     [Locks(A)    ]     [Locks(A, B) ]
    //
    //     [Intention 01] --> [Intention 03]
    //     [Locks(B)    ]     [Locks(B)    ]
    //
    // e.g. Intention 00 & 01 can execute in parallel because there are no resource conflicts.
    //      But Intention 02 can not be executed until after Intention 00 is complete*
    //
    // *The algorithm for determining intention sequence is greedy and resolves its resource
    // conflict with the first intention it encounters with a conflict.

    [[maybe_unused]] size_t capacityBefore = m_dagNodes.capacity();
    for (SearchNode* searchNode : bestIntentions) {

        m_dagNodes.push_back({});
        assert(capacityBefore == m_dagNodes.capacity() && "DAG vector resized causing the open list to hold onto invalidated pointers!");

        DAGNode *newDAGNode = &m_dagNodes.back();
        newDAGNode->node    = searchNode;

        if (searchNode->isDelegation()) {
            m_open.push_back(newDAGNode);
            continue;
        }

        // Easy case:
        // There are no DAG nodes to consider for resource conflicts yet
        //
        // or
        //
        // The new node has no resources it needs to lock, so there are no possible
        // resource conflicts to consider when scheduling.
        if (m_open.empty() || newDAGNode->node->plan->resourceLocks().empty()) {
            m_open.push_back(newDAGNode);
            continue;
        }

        // Traverse the DAG tree to find any potential resource conflicts. We are dependent on
        // any node that uses any resource that we also want to use, i.e. we must execute after
        // they are executed, so we queue up the new DAG node as a child of the node we are
        // dependent on.
        struct DAGNodeLink
        {
            DAGNode *node;
            DAGNode *parent;
        };

        // Seed the initial queue of root DAG nodes we need to check.
        std::vector<DAGNodeLink> dependencyNodeCheckList;
        for (DAGNode *node : m_open) {
            dependencyNodeCheckList.push_back({node, nullptr /*parent*/});
        }

        while (dependencyNodeCheckList.size()) {

            // Pop the first node from the front of the list
            DAGNodeLink nextDAG = dependencyNodeCheckList.front();
            dependencyNodeCheckList.erase(dependencyNodeCheckList.begin());

            if (!hasResourceDependency(newDAGNode, nextDAG.node)) {

                if (dependencyNodeCheckList.empty()) {
                    // Easy case: No dependencies detected, add as a node to be executed in
                    // parallel alongside the active node
                    if (nextDAG.parent) {
                        nextDAG.parent->children.push_back(newDAGNode);
                    } else {
                        m_open.push_back(newDAGNode);
                    }
                    break;
                }

                // If the dependencyNodeCheckList still has nodes, we need to check that we have
                // no resource dependency on those nodes.
                continue;
            }

            // We have a resource conflict in this node. We are hence dependent on the current
            // node and can ignore the other node resource requirements.
            dependencyNodeCheckList.clear();

            // We use the same resources as the current activeDAGNode, we can not
            // execute in parallel with this node and need to be added as a dependency.
            //
            // This DAG node may additionally have dependent children with their own set
            // of resource dependencies we need to check.
            if (nextDAG.node->children.empty()) {
                // Easy case, there are no children we can conflict with. Add the DAG node
                // as a direct dependent of the conflicted node and we are done.
                nextDAG.node->children.push_back(newDAGNode);
                break;
            } else {
                for (DAGNode *childDAGNode : nextDAG.node->children) {
                    dependencyNodeCheckList.push_back({childDAGNode, nextDAG.node /*parent*/});
                }
            }
        }
    }
#endif
}

std::vector<DAGNode *> IntentionExecutionDAG::close(DAGNode *node)
{
    std::vector<DAGNode*> result;

    /// Find the DAG node in the open list and retrieve its children
    bool found = false;
    for (auto it = m_open.begin(); it != m_open.end(); ) {
        if (*it == node) {
            result = std::move(node->children);
            found  = true;
            m_open.erase(it);
            break;
        } else {
            it++;
        }
    }

    if (!found) {
        /// This node doesn't even belong in this dag graph
        /// \todo: this should not happen and needs fixing
        return result;
    }

    /// Add the node's children to the open list to signify that it's
    /// ready/currently being executed.
    m_open.insert(m_open.end(), result.begin(), result.end());
    return result;
}

}} // namespace aos::jack

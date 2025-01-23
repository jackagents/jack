import { CBDIIntentionBuilder } from 'main/beBuilders/cbdi/intentionBuilder/cbdiIntentionBuilderNonFlatBuffer';
import { NodeType, DelegationStatus, BDILogGoalIntentionResult, DelegationBody } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { CBDIAgent, CBDIService, CBDITeam } from 'types/cbdi/cbdi-models-non-flatbuffer';
import {
  CBDIEditorProject,
  CBDIEditorProjectAgent,
  CBDIEditorProjectGoal,
  CBDIEditorProjectRole,
  CBDIEditorProjectTeam,
} from 'misc/types/cbdiEdit/cbdiEditModel';
import { getObjectByModuleConcepts } from 'misc/utils/cbdiEdit/Helpers';
import { areObjectsEqual, removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';

const getRoleObjArrayByTemplateName = (
  project: CBDIEditorProject,
  agentObjArray: (CBDIEditorProjectTeam | CBDIEditorProjectAgent)[],
  templateType: string | undefined,
) => {
  if (templateType === undefined || !templateType.includes(' Template')) {
    return [];
  }
  const templateName = templateType.replace(/ Template$/, '');

  const agentObj = agentObjArray.find((obj) => `${obj.module}.${obj.name}` === templateName);
  if (agentObj) {
    return getObjectByModuleConcepts(project, agentObj.roles) as CBDIEditorProjectRole[];
  }
  return [];
};

const findCommonRoles = (array1: CBDIEditorProjectRole[], array2: CBDIEditorProjectRole[]): CBDIEditorProjectRole[] =>
  array1.filter((item1) => array2.some((item2) => item2.uuid === item1.uuid));

function findMatchingGoalDelegations(delegations: DelegationBody[] | undefined, goalObjs: CBDIEditorProjectGoal[]): DelegationBody[] {
  const matchingDelegations: DelegationBody[] = [];
  if (delegations === undefined) {
    return matchingDelegations;
  }
  delegations.forEach((delegation) => {
    const goalObj = goalObjs.find((item) => `${item.module}.${item.name}` === delegation.goal);
    if (goalObj) {
      matchingDelegations.push(delegation);
    }
  });
  return matchingDelegations;
}

// create graph data for runtime explainability
export const createExplainabilityGraphData = (
  project: CBDIEditorProject,
  nodeModelArr: (CBDIAgent | CBDIService)[],
  INTENTION_BUILDER: CBDIIntentionBuilder,
) => {
  const nodes: any[] = [];
  const edges: any[] = [];
  const allAgentObjs = [...project.agents, ...project.teams].map((moduleConcept) => project.cbdiObjects[moduleConcept.uuid]) as (
    | CBDIEditorProjectTeam
    | CBDIEditorProjectAgent
  )[];
  nodeModelArr.forEach((nodeModel) => {
    const { id } = nodeModel.address;

    const type = (() => {
      switch (nodeModel.address.type) {
        case NodeType.AGENT:
          return CBDIEditorRootConceptType.AgentConceptType;
        case NodeType.SERVICE:
          return CBDIEditorRootConceptType.ServiceConceptType;
        case NodeType.TEAM:
          return CBDIEditorRootConceptType.TeamConceptType;
        default:
          return CBDIEditorRootConceptType.AgentConceptType;
      }
    })();

    // if it is team node, create role and link to role and team member
    if (nodeModel.address.type === NodeType.TEAM) {
      const teamNodeModel = nodeModel as CBDITeam;
      const teamRoles = getRoleObjArrayByTemplateName(project, allAgentObjs, teamNodeModel.templateType);

      if (teamNodeModel.members) {
        teamNodeModel.members.forEach((member) => {
          const memberNodeModel = nodeModelArr.find((node) => areObjectsEqual(node.address, member)) as CBDIAgent;
          if (memberNodeModel) {
            const delegationBodys = memberNodeModel.delegations?.filter((item) => item.teamId === nodeModel.address.id);

            const memberRoles = getRoleObjArrayByTemplateName(project, allAgentObjs, memberNodeModel.templateType);

            const commonRoles = findCommonRoles(teamRoles, memberRoles);
            commonRoles.forEach((roleObj) => {
              const delegationDic: {
                [goalName: string]: any[];
              } = {};
              const goalObjs = getObjectByModuleConcepts(project, roleObj.goals) as CBDIEditorProjectGoal[];
              const matchingDelegationBodys = findMatchingGoalDelegations(delegationBodys, goalObjs);
              // create role node
              const roleNodeId = `${id}::${roleObj.uuid}`;
              let roleNode = nodes.find((el) => el.id === roleNodeId);
              // if roleNode does not exist yet, create one
              if (roleNode === undefined) {
                roleNode = {
                  id: roleNodeId,
                  data: {
                    type: CBDIEditorRootConceptType.RoleConceptType,
                    label: roleObj.name,
                  },
                };
                nodes.push(roleNode);
                const upLink = {
                  source: id,
                  target: roleNodeId,
                };
                edges.push(upLink);
              }
              // create goal group node
              const goalGroupNodeId = `${id}::${roleNodeId}::${member.id}`;
              let goalGroupNode = nodes.find((el) => el.id === goalGroupNodeId);
              if (goalGroupNode === undefined) {
                goalGroupNode = {
                  id: goalGroupNodeId,
                  data: {
                    type: CBDIEditorRootConceptType.GoalConceptType,
                    label: `goal group for ${goalGroupNodeId}`,
                    children: [],
                  },
                };
                nodes.push(goalGroupNode);
                const upLink = {
                  source: roleNodeId,
                  target: goalGroupNodeId,
                };
                const downLink = {
                  source: goalGroupNodeId,
                  target: member.id,
                };
                edges.push(upLink, downLink);
              }
              // TODO
              // Temporary disregard the delegations for ControlRole for now
              if (matchingDelegationBodys.length > 0 && roleObj.name !== 'ControlRole' && roleObj.name !== 'JammingRole') {
                matchingDelegationBodys.forEach((delegation) => {
                  const matchedDelegation = memberNodeModel.delegations?.find((el) => el.goalId === delegation.goalId);
                  const delegationStatus = (() => {
                    if (matchedDelegation === undefined) {
                      return 'Pending';
                    }
                    if (matchedDelegation.status === DelegationStatus.PENDING) {
                      return 'Pending';
                    }

                    if (matchedDelegation.status === DelegationStatus.FAILED) {
                      return 'Failed';
                    }
                    if (matchedDelegation.status === DelegationStatus.SUCCESS) {
                      return 'Success';
                    }
                    return 'Pending';
                  })();
                  const intentionStatus = INTENTION_BUILDER.getIntentionStatusBySubGoalIdDelegatingAgent(member.id, delegation.goalId);
                  // change pending to in progress for delegation node
                  // TODO
                  // Temporary Hack
                  // when delegation status is pending, use intention finish status
                  const delegationStatusLabel = (() => {
                    if (delegationStatus === 'Pending') {
                      switch (intentionStatus) {
                        case BDILogGoalIntentionResult.DROPPED:
                          return 'Dropped';
                        case BDILogGoalIntentionResult.FAILED:
                          return 'Failed';
                        case BDILogGoalIntentionResult.SUCCESS:
                          return 'Success';
                        default:
                          return 'In Progress';
                      }
                    }
                    return delegationStatus;
                  })();

                  const goalNodeData: any = {
                    type: CBDIEditorRootConceptType.GoalConceptType,
                    // remove the module prefix from goal name
                    label: `${removeBeforeFirstDotAndDot(delegation.goal)} (${delegationStatusLabel})`,
                    subGoalId: delegation.goalId,
                    delegationStatus,
                    delegation,
                    delegatingAgentAddress: memberNodeModel.address,
                  };
                  // limit to 3 delegations for each agent, each role, each goal name

                  // const delegationLength =
                  //   delegationDic[delegation.goal]?.length;
                  // if (
                  //   delegationLength === undefined ||
                  //   delegationLength === 0
                  // ) {
                  //   goalNode.data.opacity = 1;
                  //   delegationDic[delegation.goal] = [goalNode];
                  // } else if (delegationLength === 1) {
                  //   delegationDic[delegation.goal][0].data.opacity = 0.6;
                  //   goalNode.data.opacity = 1;
                  //   delegationDic[delegation.goal].push(goalNode);
                  // } else if (delegationLength === 2) {
                  //   delegationDic[delegation.goal][0].data.opacity = 0.3;
                  //   delegationDic[delegation.goal][1].data.opacity = 0.6;
                  //   goalNode.data.opacity = 1;
                  //   delegationDic[delegation.goal].push(goalNode);
                  // } else {
                  //   delegationDic[delegation.goal].splice(
                  //     0,
                  //     delegationDic[delegation.goal].length - 2
                  //   );
                  //   delegationDic[delegation.goal][0].data.opacity = 0.3;
                  //   delegationDic[delegation.goal][1].data.opacity = 0.6;
                  //   goalNode.data.opacity = 1;
                  //   delegationDic[delegation.goal].push(goalNode);
                  // }

                  // limit to 1 delegations for each agent, each role, each goal name
                  goalNodeData.opacity = 1;
                  delegationDic[delegation.goal] = [goalNodeData];
                });
              }
              Object.values(delegationDic).forEach((goalNodeDataArr) => {
                goalNodeDataArr.forEach((goalNodeData) => {
                  goalGroupNode.data.children.push(goalNodeData);
                });
              });
            });
          }
        });
      }
    }

    // only render agent/team/role node for now
    if (nodeModel.address.type === NodeType.AGENT || nodeModel.address.type === NodeType.TEAM) {
      const node = {
        id,
        data: {
          type,
          label: nodeModel.address.name,
          agentBusAddress: nodeModel.address,
        },
      };
      nodes.push(node);
    }
  });

  return { nodes, edges };
};

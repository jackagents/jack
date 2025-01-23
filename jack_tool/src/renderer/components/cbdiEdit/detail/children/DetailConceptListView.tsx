import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import { Fluid, List } from 'components/common/base/BaseContainer';
import { SelectChangeEvent, styled } from '@mui/material';
import {
  getObjectByModuleConcept,
  copy,
  sortModuleConceptList,
  getAllObjOptionsForSingleItem,
  getAddingModuleConceptOptions,
} from 'misc/utils/cbdiEdit/Helpers';
import ForwardIcon from '@mui/icons-material/Forward';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import { ThemedSelect, ThemedMenuItem } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { CBDIEditorProjectAgent, CBDIEditorProjectPlan, CBDIEditorProjectRole, CBDIEditorProjectTeam } from 'misc/types/cbdiEdit/cbdiEditModel';
import {
  ModuleConcept,
  ProjectConceptListType,
  ConceptFieldType,
  CBDIEditorObject,
  Mod,
  CBDIEditorRootConceptType,
  CBDIEditorSharedMessage,
  EmptyModuleConcept,
} from 'misc/types/cbdiEdit/cbdiEditTypes';
import DetailConceptListItem from './DetailConceptListItem';
import RoleDetailMessageListItem from '../conceptDetail/role/RoleDetailMessageListItem';

/* --------------------------------- Styles --------------------------------- */

const ConceptList = styled(List)(({ theme }) => ({
  overflow: 'hidden',
  '& > *:first-of-type': {
    borderTop: `1px solid ${theme.editor.detailView.textColor}`,
  },
}));

/* -------------------------- DetailConceptListView ------------------------- */

interface Props {
  moduleConcept: ModuleConcept;
  addingItemType: ProjectConceptListType;
  addingItemField: ConceptFieldType;
}

function DetailConceptListView({ moduleConcept, addingItemType, addingItemField }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* -------------------------------- Functions ------------------------------- */
  const getMenuItem = (optionModuleConcept: ModuleConcept, currentObj: CBDIEditorObject | undefined, index: number) => {
    const mobject = getObjectByModuleConcept(current!, optionModuleConcept);
    const prefix = `${optionModuleConcept.module}::`;
    if (mobject && mobject._mod !== Mod.Deletion) {
      // if it is adding plan to an agent/team
      // put goal suffix that plan handles
      if (
        (currentObj?._objectType === CBDIEditorRootConceptType.AgentConceptType ||
          currentObj?._objectType === CBDIEditorRootConceptType.TeamConceptType) &&
        addingItemType === 'plans'
      ) {
        const handleGoalModuleConcept = (mobject as CBDIEditorProjectPlan).handles;

        const handleGoalObj = getObjectByModuleConcept(current, handleGoalModuleConcept);
        const goalName = (() => {
          if (!handleGoalObj) {
            return `${handleGoalModuleConcept.name}`;
          }
          return `${handleGoalObj.name}`;
        })();
        return (
          <ThemedMenuItem key={index} value={JSON.stringify(optionModuleConcept)}>
            <span style={{ display: 'flex', alignItems: 'center', gap: 5 }}>
              {mobject.name}
              <ForwardIcon style={{ fontSize: 16 }} />
              {goalName}
            </span>
          </ThemedMenuItem>
        );
      }

      if (
        (currentObj?._objectType === CBDIEditorRootConceptType.AgentConceptType ||
          currentObj?._objectType === CBDIEditorRootConceptType.TeamConceptType) &&
        addingItemType === 'roles'
      ) {
        const roleGoals = (mobject as CBDIEditorProjectRole).goals;

        let goalStr = 'no goal';

        for (let i = 0; i < roleGoals.length; i++) {
          const goalModuleConcept = roleGoals[i];
          const goalObj = getObjectByModuleConcept(current, goalModuleConcept);
          const singleGoalStr = (() => {
            if (!goalObj) {
              return `${goalModuleConcept.name}`;
            }
            return `${goalObj.name}`;
          })();
          if (i === 0) {
            goalStr = singleGoalStr;
          } else {
            goalStr = `${goalStr}, ${singleGoalStr}`;
          }
        }
        return (
          <ThemedMenuItem key={index} value={JSON.stringify(optionModuleConcept)}>
            <span
              style={{
                display: 'flex',
                alignItems: 'center',
                gap: 5,
              }}
              title={prefix + mobject.name}
            >
              {mobject.name}
              <ForwardIcon style={{ fontSize: 16 }} />
              {goalStr}
            </span>
          </ThemedMenuItem>
        );
      }
      return (
        <ThemedMenuItem key={index as number} value={JSON.stringify(optionModuleConcept)}>
          <span title={prefix + mobject.name}> {mobject.name}</span>
        </ThemedMenuItem>
      );
    }
    return null;
  };

  /* -------------------------------- Callbacks ------------------------------- */
  const onAddItem = (oldConcept: CBDIEditorObject, maddingItemTypeGroup: ConceptFieldType, moduleConceptString: string) => {
    const mmoduleConcept: ModuleConcept = JSON.parse(moduleConceptString);
    const concept = copy(oldConcept);
    let newItem: any = mmoduleConcept;
    if (oldConcept._objectType === CBDIEditorRootConceptType.RoleConceptType && maddingItemTypeGroup === 'messages') {
      newItem = {
        ...mmoduleConcept,
        write: false,
        read: false,
      } as CBDIEditorSharedMessage;
    }
    // if adding plan's handle goal is not included in agent/team 's goals, add it
    else if (
      (oldConcept._objectType === CBDIEditorRootConceptType.AgentConceptType ||
        oldConcept._objectType === CBDIEditorRootConceptType.TeamConceptType) &&
      maddingItemTypeGroup === 'plans'
    ) {
      const newPlan = getObjectByModuleConcept(current, newItem as ModuleConcept) as CBDIEditorProjectPlan;
      if (!(concept as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.some((goalItem) => areModuleConceptsEqual(goalItem, newPlan.handles))) {
        (concept as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.unshift({
          ...newPlan.handles,
          startup_goal: false,
          startup_tactic: EmptyModuleConcept,
        });
      }
    }
    // if adding role's goals are not included in agent/team 's goals, add them
    else if (
      (oldConcept._objectType === CBDIEditorRootConceptType.AgentConceptType ||
        oldConcept._objectType === CBDIEditorRootConceptType.TeamConceptType) &&
      maddingItemTypeGroup === 'roles'
    ) {
      const newRole = getObjectByModuleConcept(current, newItem as ModuleConcept) as CBDIEditorProjectRole;
      newRole.goals.forEach((goalModuleConcept) => {
        if (
          !(concept as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.some((goalItem) => areModuleConceptsEqual(goalItem, goalModuleConcept))
        ) {
          (concept as CBDIEditorProjectAgent | CBDIEditorProjectTeam).goals.unshift({
            ...goalModuleConcept,
            startup_goal: false,
            startup_tactic: EmptyModuleConcept,
          });
        }
      });
    }

    concept[maddingItemTypeGroup].unshift(newItem);
    dispatch(updateObjects(concept));
  };

  /* ------------------------------ useMemo hooks ----------------------------- */
  const defaultLabel = React.useMemo(() => {
    switch (addingItemField) {
      case 'action_handlers':
        return 'Add an action';
      case 'query_messages':
        return 'Add a query message';
      case 'actions':
        return 'Add an action';
      case 'agents':
        return 'Add an agent';
      case 'children':
        return 'Add a child entity';
      default:
        return `Add a ${addingItemType.slice(0, -1)}`;
    }
  }, [addingItemField]);

  const [object, itemsInList, moduleConceptToAdd] = React.useMemo(() => {
    const mobject = getObjectByModuleConcept(current!, moduleConcept);
    let mitemsInList: any[] = [];
    let mmoduleConceptToAdd: ModuleConcept[] = [];
    let moduleConceptsInList: ModuleConcept[] = [];

    if (mobject) {
      mitemsInList = (mobject as any)[addingItemField];
      if (mobject._objectType === CBDIEditorRootConceptType.RoleConceptType && addingItemField === 'messages') {
        moduleConceptsInList = mitemsInList.map((item: CBDIEditorSharedMessage) => ({
          module: item.module,
          name: item.name,
          uuid: item.uuid,
        }));
      } else {
        moduleConceptsInList = mitemsInList;
      }
      if (mobject._objectType === CBDIEditorRootConceptType.EntityConceptType && addingItemField === 'children') {
        mmoduleConceptToAdd = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.EntityConceptType, current!).filter(
          (item) => !areModuleConceptsEqual(item, moduleConcept),
        );
      } else {
        const addingModuleConceptOptions = getAddingModuleConceptOptions(moduleConceptsInList, addingItemType, current!);
        mmoduleConceptToAdd = [...new Set(addingModuleConceptOptions)];
      }
    }

    // sort itemsInList and moduleConceptToAdd
    mitemsInList = sortModuleConceptList(mitemsInList, current!);
    mmoduleConceptToAdd = sortModuleConceptList(mmoduleConceptToAdd, current!);
    return [mobject, mitemsInList, mmoduleConceptToAdd];
  }, [current, moduleConcept]);

  /* ------------------------------- Components ------------------------------- */
  return (
    <Fluid>
      <ThemedSelect
        value=""
        label={defaultLabel}
        onChange={(event: SelectChangeEvent<unknown>) => {
          onAddItem(object!, addingItemField, event.target.value as string);
        }}
      >
        <ThemedMenuItem disabled value="">
          {defaultLabel || 'Add'}
        </ThemedMenuItem>
        {moduleConceptToAdd.map((mmoduleConcept: ModuleConcept, index: number) => getMenuItem(mmoduleConcept, object, index))}
      </ThemedSelect>
      <ConceptList>
        {itemsInList.map((item: any, index: number) => {
          if (object?._objectType === CBDIEditorRootConceptType.RoleConceptType && addingItemField === 'messages') {
            const roleMsg = item as CBDIEditorSharedMessage;
            return <RoleDetailMessageListItem key={index as number} roleObj={object!} roleMsg={roleMsg} />;
          }
          return <DetailConceptListItem key={index as number} moduleConcept={item} parentObj={object!} addingItemField={addingItemField} />;
        })}
      </ConceptList>
    </Fluid>
  );
}

export default React.memo(DetailConceptListView);

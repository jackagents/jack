import React from 'react';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { getAddingModuleConceptOptions, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { v4 } from 'uuid';
import { DEFAULT_PLAN_POLICY } from 'constant/cbdiEdit/cbdiEditConstant';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorModuleConceptWithId, ModuleConcept, PlanOrderType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectTactic, CBDIEditorProjectPlan } from 'misc/types/cbdiEdit/cbdiEditModel';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import PlanPolicy from './planPolicy/PlanPolicy';

export default function TacticPlanPolicy({ tacticModuleConcept }: { tacticModuleConcept: ModuleConcept }) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const [tacticObj, allPlanList] = React.useMemo(() => {
    const mtacticObj = getObjectByModuleConcept(current!, tacticModuleConcept) as CBDIEditorProjectTactic | undefined;

    const allPlanReferConcepts: ModuleConcept[] = getAddingModuleConceptOptions([], 'plans', current!);
    const mallPlanList = allPlanReferConcepts.filter((planReferConcept) => {
      const planObj = getObjectByModuleConcept(current!, planReferConcept) as CBDIEditorProjectPlan | undefined;
      if (areModuleConceptsEqual(mtacticObj?.goal, planObj?.handles)) {
        return true;
      }
      return false;
    });

    return [mtacticObj, mallPlanList];
  }, [current, tacticModuleConcept]);

  /* -------------------------------- Callbacks ------------------------------- */
  const onToggleUserPlanList = () => {
    if (tacticObj) {
      const newTacticObj: CBDIEditorProjectTactic = {
        ...tacticObj,
        use_plan_list: !tacticObj.use_plan_list,
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  const onTogglePlanOrder = (planOrder: PlanOrderType) => {
    if (tacticObj) {
      const newTacticObj: CBDIEditorProjectTactic = {
        ...tacticObj,
        plan_order: planOrder,
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  const onChangePlanLoop = (planLoop: number) => {
    if (tacticObj) {
      const newTacticObj: CBDIEditorProjectTactic = {
        ...tacticObj,
        plan_loop: planLoop,
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  const onAddPlanItem = (planModuleConcept: ModuleConcept) => {
    if (tacticObj) {
      const policyPlanListItem = { id: v4(), moduleConcept: planModuleConcept };
      const newTacticObj: CBDIEditorProjectTactic = {
        ...tacticObj,
        plan_list: [...tacticObj.plan_list, policyPlanListItem],
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  const onRemovePlanItem = (deletingId: string) => {
    if (tacticObj) {
      const newPlanList = tacticObj.plan_list.filter((item) => item.id !== deletingId);
      const newTacticObj: CBDIEditorProjectTactic = {
        ...tacticObj,
        plan_list: newPlanList,
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  const onReorderPlanList = (newPlanList: CBDIEditorModuleConceptWithId[]) => {
    if (tacticObj) {
      const newTacticObj: CBDIEditorProjectTactic = {
        ...tacticObj,
        plan_list: newPlanList,
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  const onDefaultPlanPolicy = () => {
    if (tacticObj) {
      const newTacticObj: CBDIEditorProjectTactic = {
        ...tacticObj,
        ...DEFAULT_PLAN_POLICY,
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  /* ------------------------------- Components ------------------------------- */
  if (!tacticObj) {
    return null;
  }

  return (
    <PlanPolicy
      immutable={false}
      project={current!}
      tactic={tacticObj}
      allPlanList={allPlanList}
      textColor="black"
      onToggleUserPlanList={onToggleUserPlanList}
      onTogglePlanOrder={onTogglePlanOrder}
      onChangePlanLoop={onChangePlanLoop}
      onAddPlanItem={onAddPlanItem}
      onRemovePlanItem={onRemovePlanItem}
      onReorderPlanList={onReorderPlanList}
      onDefaultPlanPolicy={onDefaultPlanPolicy}
    />
  );
}

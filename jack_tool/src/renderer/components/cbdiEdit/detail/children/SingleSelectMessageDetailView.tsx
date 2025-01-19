import React from 'react';
import { SelectChangeEvent } from '@mui/material';
import { getAllObjOptionsForSingleItem, getObjectByModuleConcept, copy, sortModuleConceptList } from 'misc/utils/cbdiEdit/Helpers';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorObject, CBDIEditorRootConceptType, EmptyModuleConcept, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectAction, CBDIEditorProjectGoal } from 'misc/types/cbdiEdit/cbdiEditModel';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';

/* ------------------------ SingleSelectMessageDetail ----------------------- */
interface Props {
  disabled?: boolean;
  oldObj: CBDIEditorObject;
  conceptType: 'action_req' | 'action_rpy' | 'goal_message' | 'action_feedback';
  currentConcept: ModuleConcept;
}

function SingleSelectMessageDetail(props: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const messageOptions = React.useMemo(() => {
    const unSortedMessageOptions = getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.MessageConceptType, current!);

    const sortedMessageOptions = sortModuleConceptList(unSortedMessageOptions, current!);
    return sortedMessageOptions;
  }, [current]);

  /* -------------------------------- Callbacks ------------------------------- */

  const onChange = (oldObj: any, moduleConceptId: string) => {
    const obj = copy(oldObj);
    const moduleConcept = messageOptions.find((el) => el.uuid === moduleConceptId);
    if (moduleConcept) {
      switch (props.conceptType) {
        case 'action_req':
          (obj as CBDIEditorProjectAction).request = moduleConcept;
          break;
        case 'action_rpy':
          (obj as CBDIEditorProjectAction).reply = moduleConcept;
          break;
        case 'action_feedback':
          (obj as CBDIEditorProjectAction).feedback = moduleConcept;
          break;
        case 'goal_message':
          (obj as CBDIEditorProjectGoal).message = moduleConcept;
          break;
        default:
          break;
      }
    } else {
      switch (props.conceptType) {
        case 'action_req':
          (obj as CBDIEditorProjectAction).request = EmptyModuleConcept;
          break;
        case 'action_rpy':
          (obj as CBDIEditorProjectAction).reply = EmptyModuleConcept;
          break;
        case 'action_feedback':
          (obj as CBDIEditorProjectAction).feedback = EmptyModuleConcept;
          break;
        case 'goal_message':
          (obj as CBDIEditorProjectGoal).message = EmptyModuleConcept;
          break;
        default:
          break;
      }
    }
    dispatch(updateObjects(obj));
  };

  /* ------------------------------- Components ------------------------------- */
  return (
    <ThemedSelect
      hasborderbottom="true"
      disabled={props.disabled}
      value={props.currentConcept.uuid}
      defaultValue={EmptyModuleConcept.uuid}
      onChange={(event: SelectChangeEvent<unknown>) => {
        onChange(props.oldObj, event.target.value as string);
      }}
    >
      <ThemedMenuItem value={EmptyModuleConcept.uuid}>Select a message</ThemedMenuItem>
      {messageOptions.map((moduleConcept, index) => {
        const object = getObjectByModuleConcept(current!, moduleConcept);
        const prefix = `${moduleConcept.module}::`;
        if (object) {
          const isDeleted = object._mod === Mod.Deletion;

          if (isDeleted && !areModuleConceptsEqual(moduleConcept, props.currentConcept)) {
            return null;
          }
          return (
            <ThemedMenuItem key={index as number} value={moduleConcept.uuid} disabled={isDeleted}>
              <span className={isDeleted ? 'editor-missing' : undefined} title={prefix + object.name}>
                {object.name}
              </span>
            </ThemedMenuItem>
          );
        }
        return null;
      })}
      {!areModuleConceptsEqual(props.currentConcept, EmptyModuleConcept) && !getObjectByModuleConcept(current!, props.currentConcept) ? (
        <ThemedMenuItem value={props.currentConcept.uuid} disabled>
          <span className="editor-missing">{props.currentConcept.name}</span>
        </ThemedMenuItem>
      ) : null}
    </ThemedSelect>
  );
}

export default React.memo(SingleSelectMessageDetail);

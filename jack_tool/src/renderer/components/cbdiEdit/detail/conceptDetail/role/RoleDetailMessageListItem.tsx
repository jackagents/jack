/* eslint-disable no-param-reassign */
import React from 'react';
import { styled } from '@mui/material';
import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { Between, FlexCol } from 'components/common/base/BaseContainer';
import {
  Report as ReportIcon,
  Delete as DeleteIcon,
  CheckBoxOutlineBlank as CheckBoxUncheckedIcon,
  CheckBox as CheckBoxCheckedIcon,
} from '@mui/icons-material';
import { copy, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { ThemedButton } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { RootState } from 'projectRedux/Store';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';
import { CBDIEditorObject, CBDIEditorSharedMessage, Mod } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectRole } from 'misc/types/cbdiEdit/cbdiEditModel';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('li')({
  width: '100%',
  backgroundColor: 'transparent',
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
});

const TextIconContainer = styled('div')({
  display: 'flex',
  paddingLeft: 5,
  alignItems: 'center',
  gap: 10,
  whiteSpace: 'nowrap',
  overflow: 'hidden',
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontSize: '.9em',
  overflow: 'hidden',
  textOverflow: 'ellipsis ',
});

const TextSlot = styled(FlexCol)({
  justifyContent: 'center',
});

const RemoveButton = styled('button')(({ theme }) => ({
  padding: 0,
  backgroundColor: 'transparent',
  color: theme.editor.detailView.textColor,
}));

const CheckboxContainer = styled('div')({
  display: 'flex',
  gap: 8,
  fontSize: '.9em',
  alignItems: 'center',
});

/* -------------------------- RoleDetailMessageListItem ------------------------- */

interface Props {
  roleObj: CBDIEditorObject;
  roleMsg: CBDIEditorSharedMessage;
}

function RoleDetailMessageListItem({ roleMsg, roleObj }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;
  /* ------------------------------ useMeno hooks ----------------------------- */
  const [object, prefix] = React.useMemo(() => {
    const mobject = getObjectByModuleConcept(current!, roleMsg);
    const mprefix = mobject ? `${mobject.module}::` : `${roleMsg.module}::`;
    return [mobject, mprefix];
  }, [current, roleMsg]);

  /* -------------------------------- Callbacks ------------------------------- */
  const onRemoveItem = () => {
    const newRoleObj = copy(roleObj) as CBDIEditorProjectRole;
    newRoleObj.messages.forEach((i: CBDIEditorSharedMessage, index: number) => {
      if (areModuleConceptsEqual(i, roleMsg)) {
        newRoleObj.messages.splice(index, 1);
      }
    });
    dispatch(updateObjects(newRoleObj));
  };

  const onToggle = (toggleField: 'read' | 'write') => {
    const newRoleObj = copy(roleObj) as CBDIEditorProjectRole;
    newRoleObj.messages.forEach((i: CBDIEditorSharedMessage) => {
      if (areModuleConceptsEqual(i, roleMsg)) {
        i[toggleField] = !i[toggleField];
      }
    });
    dispatch(updateObjects(newRoleObj));
  };
  /* ------------------------------- Components ------------------------------- */
  return (
    <Root>
      <Between>
        <TextIconContainer>
          {object && object._mod !== Mod.Deletion ? (
            <img alt="" src={nodeIcon[object._objectType]} style={{ width: 22, height: 22, padding: 2 }} />
          ) : (
            <ReportIcon style={{ color: '#d40b0b', padding: 3 }} />
          )}
          <TextView
            title={object ? prefix + object.name : prefix + roleMsg.name}
            className={!object || object._mod === Mod.Deletion ? 'editor-missing' : undefined}
          >
            {object ? object.name : roleMsg.name}
          </TextView>
        </TextIconContainer>
        <TextIconContainer style={{ flexShrink: 0 }}>
          <CheckboxContainer>
            <ThemedButton
              onClick={() => {
                onToggle('read');
              }}
            >
              {roleMsg.read ? <CheckBoxCheckedIcon /> : <CheckBoxUncheckedIcon />}
            </ThemedButton>
            <TextSlot>Read</TextSlot>
          </CheckboxContainer>
          <CheckboxContainer>
            <ThemedButton
              onClick={() => {
                onToggle('write');
              }}
            >
              {roleMsg.write ? <CheckBoxCheckedIcon /> : <CheckBoxUncheckedIcon />}
            </ThemedButton>
            <TextSlot>Write</TextSlot>
          </CheckboxContainer>
          <RemoveButton onClick={onRemoveItem}>
            <DeleteIcon style={{ padding: 2 }} />
          </RemoveButton>
        </TextIconContainer>
      </Between>
    </Root>
  );
}

export default React.memo(RoleDetailMessageListItem);

/* eslint-disable no-underscore-dangle */
import React, { useMemo } from 'react';
import { styled } from '@mui/material';
import { capitalize, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorRootConceptType, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import ToggleTeamAgentButton from './ToggleTeamAgentButton';
import Avatar from './Avatar';
import ChangeModule from './ChangeModule/ChangeModule';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  width: '100%',
  height: 60,
  position: 'relative',
});

const AvatarSlot = styled('div')({
  position: 'absolute',
  left: 0,
  width: 50,
  height: 50,
});

const NameAndType = styled('div')({
  position: 'absolute',
  left: 60,
  right: 0,
  height: 50,
});

const TextView = styled('div')({
  width: '100%',
  height: 25,
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  lineHeight: '25px',
  textOverflow: 'ellipsis',
  overflow: 'hidden',
  whiteSpace: 'nowrap',
});

/* ------------------------------ DetailHeader ------------------------------ */

function DetailHeader({ moduleConcept }: { moduleConcept: ModuleConcept }) {
  /* ---------------------------------- Redux --------------------------------- */
  const current: CBDIEditorProject | null = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  const dispatch = useDispatch();
  const { switchTeamAgent } = cbdiEditActions;

  /* -------------------------------- useMemo hooks -------------------------------- */

  const obj = useMemo(() => getObjectByModuleConcept(current, moduleConcept), [current, moduleConcept]);
  if (!obj || obj._mod === Mod.Deletion) {
    return null;
  }
  /* ------------------------------- Components ------------------------------- */
  return (
    <div style={{ display: 'flex', paddingRight: 5, justifyContent: 'space-between' }}>
      <Root>
        <AvatarSlot>
          <Avatar selectedTreeNodeObj={obj} />
        </AvatarSlot>
        <NameAndType>
          <TextView title={obj ? `${obj.module}::${obj.name}` : ''} style={{ fontWeight: 'bold' }}>
            {obj ? `${obj.name}` : ''}
          </TextView>
          <TextView>{obj ? capitalize(obj._objectType) : ''}</TextView>
        </NameAndType>
      </Root>
      <div style={{ display: 'flex', gap: 20 }}>
        {obj && <ChangeModule updatingObject={obj} />}

        {obj && (obj._objectType === CBDIEditorRootConceptType.TeamConceptType || obj._objectType === CBDIEditorRootConceptType.AgentConceptType) ? (
          <ToggleTeamAgentButton
            value={obj!._objectType}
            onButtonClick={() => {
              dispatch(switchTeamAgent(moduleConcept));
            }}
          />
        ) : null}
      </div>
    </div>
  );
}

export default React.memo(DetailHeader);

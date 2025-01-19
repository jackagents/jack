import React from 'react';
import { Button, styled } from '@mui/material';
import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { Between } from 'components/common/base/BaseContainer';
import { Delete as DeleteIcon, Report as ReportIcon } from '@mui/icons-material';
import { copy, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { CBDIEditorObject, ConceptFieldType, Mod, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { areModuleConceptsEqual } from 'misc/utils/common/commonUtils';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('li')({
  width: '100%',
  backgroundColor: 'transparent',
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  whiteSpace: 'nowrap',
});

const TextIconContainer = styled('div')({
  display: 'flex',
  height: 30,
  paddingLeft: 5,
  gap: 5,
  overflow: 'hidden',
  whiteSpace: 'nowrap',
  alignItems: 'center',
});

const TextSlot = styled('div')({
  flexGrow: 1,
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'center',
  overflow: 'hidden',
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontSize: '.9em',
  textOverflow: 'ellipsis',
  overflow: 'hidden',
});

const RemoveButton = styled(Button)(({ theme }) => ({
  minWidth: 22,
  width: 22,
  height: 22,
  padding: 0,
  color: theme.editor.detailView.textColor,
}));
/* -------------------------- DetailConceptListItem ------------------------- */

interface Props {
  moduleConcept: ModuleConcept;
  parentObj?: CBDIEditorObject;
  addingItemField?: ConceptFieldType;
}

function DetailConceptListItem({ moduleConcept, parentObj, addingItemField }: Props) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;
  /* ------------------------------ useMeno hooks ----------------------------- */
  const [object, prefix] = React.useMemo(() => {
    const mobject = getObjectByModuleConcept(current, moduleConcept);
    const mprefix = mobject ? `${mobject.module}::` : `${moduleConcept.module}::`;
    return [mobject, mprefix];
  }, [current, moduleConcept]);

  /* -------------------------------- Callbacks ------------------------------- */
  const onRemoveItem = () => {
    const newObj = copy(parentObj);
    const removingIndex = newObj[addingItemField!].findIndex((i: ModuleConcept) => areModuleConceptsEqual(i, moduleConcept));
    if (removingIndex > -1) {
      newObj[addingItemField!].splice(removingIndex, 1);
    }

    dispatch(updateObjects(newObj));
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
          <TextSlot>
            <TextView
              title={object ? prefix + object.name : prefix + moduleConcept.name}
              className={!object || object._mod === Mod.Deletion ? 'editor-missing' : undefined}
            >
              {object ? object.name : moduleConcept.name}
            </TextView>
          </TextSlot>
        </TextIconContainer>
        {parentObj && addingItemField ? (
          <RemoveButton onClick={onRemoveItem}>
            <DeleteIcon style={{ padding: 2 }} />
          </RemoveButton>
        ) : null}
      </Between>
    </Root>
  );
}

export default React.memo(DetailConceptListItem);

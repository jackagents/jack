/* eslint-disable @typescript-eslint/no-non-null-assertion */
import React from 'react';
import { Grid, styled } from '@mui/material';
import TextareaAutosize from '@mui/material/TextareaAutosize';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { copy, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { isModuleConceptOverview } from 'misc/utils/common/commonUtils';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorObject } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* --------------------------------- styles --------------------------------- */
const ContainerGrid = styled(Grid)({
  width: 340,
  height: 195,
  padding: 20,
});

function NotesEditView() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* ----------------------------- useState hooks ----------------------------- */
  const [note, setNote] = React.useState<string>('');
  /* ----------------------------- useEffect hooks ---------------------------- */
  React.useEffect(() => {
    if (current && selectedTreeNodeConcept) {
      const selectedObj = getObjectByModuleConcept(current, selectedTreeNodeConcept);
      if (selectedObj) {
        setNote(selectedObj.note);
      }
    }
  }, [selectedTreeNodeConcept, current]);

  /* -------------------------------- Functions ------------------------------- */
  const getCurrentNodeName = () => {
    const selectedGraphConcept = selectedTreeNodeConcept!;
    if (current && selectedGraphConcept) {
      if (isModuleConceptOverview(selectedGraphConcept)) {
        return selectedGraphConcept.name === '' ? current.name : selectedGraphConcept.name;
      }
      const currentNode = getObjectByModuleConcept(current, selectedGraphConcept);

      return currentNode?.name;
    }
    return null;
  };
  /* -------------------------------- Callbacks ------------------------------- */
  const onUpdateNote = () => {
    const processedNote = note.replace(/\n+$/, '');
    const selectedObj = getObjectByModuleConcept(current!, selectedTreeNodeConcept);
    if (selectedObj) {
      const newObj: CBDIEditorObject = copy(selectedObj);
      newObj.note = processedNote;
      dispatch(updateObjects(newObj));
    }
  };
  /* ------------------------------- Components ------------------------------- */
  return (
    <ContainerGrid>
      <div>Notes for: {getCurrentNodeName()}</div>
      <TextareaAutosize
        placeholder="Input your design note"
        value={note || ''}
        onChange={(
          event: React.ChangeEvent<{
            value: unknown;
          }>,
        ) => {
          setNote(event.target.value as string);
        }}
        onBlur={onUpdateNote}
        style={{
          width: 300,
          height: 120,
          marginTop: 10,
          resize: 'none',
          overflow: 'hidden',
          overflowY: 'scroll',
          fontSize: 16,
          fontFamily: 'sans-serif',
        }}
      />
    </ContainerGrid>
  );
}

export default React.memo(NotesEditView);

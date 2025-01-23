import { useEffect, useMemo, useState } from 'react';
import { Modal, styled } from '@mui/material';
import Select, { SingleValue } from 'react-select';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { CBDIEditorProject } from 'misc/types/cbdiEdit/cbdiEditModel';
import { CBDIEditorObject } from 'misc/types/cbdiEdit/cbdiEditTypes';

const StyledButton = styled('button')({
  fontSize: 12,
  padding: 5,
  whiteSpace: 'nowrap',
  outline: '1px solid black',
});
function ChangeModule({ updatingObject }: { updatingObject: CBDIEditorObject }) {
  /* ---------------------------------- Redux --------------------------------- */
  const current: CBDIEditorProject | null = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);

  const dispatch = useDispatch();
  const { updateObjectModule } = cbdiEditActions;

  /* ----------------------------- useState hooks ----------------------------- */
  const [isChangingModule, setIsChangingModule] = useState(false);
  const [currentModule, setCurrentModule] = useState<string>(updatingObject.module);
  /* ----------------------------- useMemo hooks ----------------------------- */
  const moduleOptions = useMemo(() => {
    if (current) {
      return current.modulePaths.map((el) => ({ value: el.name, label: el.name })).sort((a, b) => a.label.localeCompare(b.label));
    }
    return [];
  }, [current]);

  /* ----------------------------- callbacks ----------------------------- */
  const onChangeModule = (
    option: SingleValue<{
      value: string;
      label: string;
    }>,
  ) => {
    if (option) {
      setCurrentModule(option.value);
    }
  };
  /* ----------------------------- useEffect hooks ----------------------------- */

  // Set current module to be updatingObject module when open change module modal
  useEffect(() => {
    if (isChangingModule) {
      setCurrentModule(updatingObject.module);
    }
  }, [updatingObject.module, isChangingModule]);

  return (
    <div style={{ display: 'flex', alignItems: 'center', gap: 5 }}>
      <div style={{ whiteSpace: 'nowrap' }}>{`Module: ${updatingObject.module}`}</div>
      <StyledButton
        type="button"
        onClick={() => {
          setIsChangingModule(true);
        }}
      >
        Change Module
      </StyledButton>
      <Modal
        open={isChangingModule}
        onClose={() => {
          setIsChangingModule(false);
        }}
      >
        <div
          style={{
            backgroundColor: 'white',
            position: 'absolute',
            top: '50%',
            left: '50%',
            transform: 'translate(-50%,-50%)',
            width: 400,
            minHeight: 200,
            display: 'flex',
            alignItems: 'center',
            flexDirection: 'column',
            padding: 20,
            gap: 30,
            borderRadius: 20,
          }}
        >
          <div style={{ fontSize: 20 }}>Change Module</div>
          <div
            style={{
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              gap: 50,
            }}
          >
            <div>Select Module</div>
            <Select
              styles={{
                control: (provided) => ({ ...provided, width: 200 }),
              }}
              defaultValue={{ value: updatingObject.module, label: updatingObject.module }}
              options={moduleOptions}
              onChange={onChangeModule}
            />
          </div>
          <div style={{ display: 'flex', gap: 50 }}>
            <StyledButton
              type="button"
              disabled={updatingObject.module === currentModule}
              onClick={() => {
                if (updatingObject.module !== currentModule) {
                  dispatch(updateObjectModule({ updatingModuleConcept: updatingObject, newModuleName: currentModule }));
                }
                setIsChangingModule(false);
              }}
            >
              Confirm
            </StyledButton>
            <StyledButton
              type="button"
              onClick={() => {
                setIsChangingModule(false);
              }}
            >
              Cancel
            </StyledButton>
          </div>
        </div>
      </Modal>
    </div>
  );
}

export default ChangeModule;

import React from 'react';
import { styled, Button } from '@mui/material';
import ErrorIcon from '@mui/icons-material/CancelOutlined';
import WarningIcon from '@mui/icons-material/ReportProblemOutlined';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';

/* --------------------------------- Styles --------------------------------- */
const StatusBarButton = styled(Button)({
  height: '100%',
  padding: '0 0.5em',
  '&.checked': {
    backgroundColor: '#ffffff50',
  },
});
/* ------------------------------ DebugConsoleButton ----------------------------- */

function DebugConsoleButton() {
  /* ---------------------------------- Redux --------------------------------- */
  const isVisible = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.debugConsole.isVisible);
  const errors = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.errors);
  const warnings = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.warnings);
  const dispatch = useDispatch();
  const { setDebugConsoleIsVisible } = cbdiEditActions;

  /* -------------------------------- Callbacks ------------------------------- */
  const toggleDebugConsoleIsVisible = () => {
    dispatch(setDebugConsoleIsVisible(!isVisible));
  };

  /* ------------------------------- Components ------------------------------- */

  return (
    <StatusBarButton className={isVisible ? 'checked' : ''} onClick={toggleDebugConsoleIsVisible}>
      <ErrorIcon style={{ fontSize: '1.2em', paddingRight: 2 }} />
      <span>{errors.length}</span>
      <WarningIcon style={{ fontSize: '1.3em', padding: '0 2px' }} />
      <span>{warnings.length}</span>
    </StatusBarButton>
  );
}

export default React.memo(DebugConsoleButton);

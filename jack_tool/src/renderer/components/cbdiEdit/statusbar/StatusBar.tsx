import React from 'react';
import { Fluid } from 'components/common/base/BaseContainer';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import { areObjectsEqual } from 'misc/utils/common/commonUtils';
import { RootState } from 'projectRedux/Store';
import DebugConsoleButton from './DebugConsoleButton';
/* -------------------------------- StatusBar ------------------------------- */

function StatusBar() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const isVisible = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.debugConsole.isVisible);
  const saved = useSelector((state: RootState) => state.cbdiEdit.cbdiEditSavedProject.saved);
  const dispatch = useDispatch();
  const { setDebugConsoleIsVisible } = cbdiEditActions;

  /* ------------------------------ useMemo hooks ----------------------------- */
  const backgroundColor = React.useMemo(() => {
    if ((saved && current) === null) {
      return '#404040';
    }
    if (!areObjectsEqual(saved, current)) {
      return '#e09e58';
    }
    return '#068cfa';
  }, [current, saved]);

  /* -------------------------------- Callbacks ------------------------------- */
  const handleDoubleClick = () => {
    dispatch(setDebugConsoleIsVisible(!isVisible));
  };
  /* ------------------------------- Components ------------------------------- */
  return (
    <Fluid
      style={{
        backgroundColor,
      }}
      onDoubleClick={handleDoubleClick}
    >
      <Fluid hidden={current === null}>
        <DebugConsoleButton />
      </Fluid>
    </Fluid>
  );
}

export default React.memo(StatusBar);

import React from 'react';
import { styled } from '@mui/material';
import { Fluid, List } from 'components/common/base/BaseContainer';
import { useDispatch, useSelector } from 'react-redux';
import { IModelError, IModelWarning } from 'types/cbdiEdit/cbdiEditModel';
import { RootState } from 'projectRedux/Store';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import Error from './Error';
import Warning from './Warning';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)(({ theme }) => ({
  borderTop: '2px solid #353535',
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  fontWeight: 'lighter',
  fontSize: '.9em',
  borderLeft: '4px solid black',
  backgroundColor: theme.editor.consoleView.bgColor,
  color: theme.editor.consoleView.textColor,
}));

const TabRow = styled('div')(({ theme }) => ({
  width: '100%',
  height: 30,
  backgroundColor: theme.editor.menu.bgColor,
  color: theme.editor.menu.textColor,
}));

const Tab = styled('div')({
  display: 'inline-block',
  textAlign: 'center',
  height: 30,
  minWidth: 100,
  fontSize: 12,
  fontWeight: 'lighter',
  lineHeight: '30px',
  padding: '0 1em',
});

const Content = styled('div')({
  position: 'absolute',
  inset: '30px 0 0 0',
  overflow: 'overlay',
  '&::-webkit-scrollbar': {
    width: 5,
    height: 5,
  },
  '&::-webkit-scrollbar-track': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb': {
    background: '#ffffff30',
    borderRadius: '2px',
  },
  '&::-webkit-scrollbar-corner': {
    background: 'transparent',
  },
  '&::-webkit-scrollbar-thumb:hover': {
    background: '#ffffff50',
  },
});

/* --------------------------------- DebugConsole -------------------------------- */

function DebugConsole() {
  /* ---------------------------------- Redux --------------------------------- */
  const errors = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.errors);
  const warnings = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.warnings);
  const isVisible = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.debugConsole.isVisible);
  const dispatch = useDispatch();
  const { setDebugConsoleIsVisible } = cbdiEditActions;

  /* ------------------------------- Components ------------------------------- */
  return (
    <Root>
      <TabRow
        onDoubleClick={() => {
          dispatch(setDebugConsoleIsVisible(!isVisible));
        }}
      >
        <Tab>PROBLEMS</Tab>
      </TabRow>
      <Content>
        <List>
          {errors.map((error: IModelError, index: number) => (
            <Error key={index as number} heightPx={25} error={error} />
          ))}
          {warnings.map((warning: IModelWarning, index: number) => (
            <Warning key={index as number} warning={warning} />
          ))}
        </List>
      </Content>
    </Root>
  );
}

export default React.memo(DebugConsole);

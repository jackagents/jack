import React from 'react';
import { Button, styled, Tooltip } from '@mui/material';
import {
  KeyboardTab as KeyboardTabIcon,
  Timeline as TimelineIcon,
  NoteAlt as NoteAltIcon,
} from '@mui/icons-material';
import { getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import NotesEditView from 'components/cbdiEdit/notesEditView/NotesEditView';
import { useSelector } from 'react-redux';
import { RootState } from 'projectRedux/Store';
import { CBDIEditorObject } from 'misc/types/cbdiEdit/cbdiEditTypes';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  position: 'absolute',
  width: '100%',
  height: '100%',
  top: 0,
  left: 0,
  display: 'flex',
  justifyContent: 'space-between',
  pointerEvents: 'none',
  zIndex: 99,
});

const SubContainer = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  paddingTop: 10,
  gap: 50,
  pointerEvents: 'auto',
});

const WidgetRoot = styled('div')(
  ({
    isexpanded,
    widgetwidth,
    widgetheight,
  }: {
    isexpanded: string;
    widgetwidth: number;
    widgetheight: number;
  }) => ({
    position: 'relative',
    marginBottom: isexpanded === 'true' ? widgetheight : 0,
    transition: 'all .2s ease-in-out',
    '.leftSubContainer &>div': {
      left: isexpanded === 'true' ? 0 : -widgetwidth,
      zIndex: isexpanded === 'true' ? 14 : 0,
    },
    '.rightSubContainer &>div': {
      left: isexpanded === 'true' ? -widgetwidth : 0,
      zIndex: isexpanded === 'true' ? 14 : 0,
    },
  }),
);

const WidgetContainer = styled('div')({
  position: 'absolute',
  border: '2px solid black',
  transition: 'all .2s ease-in-out',
  backgroundColor: '#dddddd',
  color: '#000000',
  borderRadius: '0 0 15px 0',
  display: 'flex',
});

const StyledButton = styled(Button)({
  position: 'absolute',
  color: 'black',
  backgroundColor: '#e09e58',
  transition: 'all .2s',
  padding: 0,
  width: 65,
  '&:hover': {
    width: 70,
    color: 'black',
    backgroundColor: '#e09e58',
  },
});

const LeftButton = styled(StyledButton)({
  borderRadius: ' 0 15px 15px 0 ',
  padding: '.5em 1em .5em 1em',
  right: -65,
  '&:hover': {
    right: -70,
  },
});

const RightButton = styled(StyledButton)({
  borderRadius: '15px 0 0 15px',
  padding: '.5em .2em .5em .5em',
  left: -65,
  '&:hover': {
    left: -70,
  },
});

const TabLeftIcon = styled(KeyboardTabIcon)({
  fontSize: 20,
  transition: 'all .5s ease-in-out',
  '&:hover': {
    transform: 'translateX(5px)',
  },
});

const TabRightIcon = styled(KeyboardTabIcon)({
  fontSize: 20,
  transform: 'rotate(-180deg)',
  '&:hover': {
    transition: 'all 0.5s ease-in-out ',
    transform: 'translateX(-5px) rotate(-180deg)',
  },
});

/* -------------------------- TooltipIconComponent -------------------------- */
const TooltipIconComponent = ({
  tooltipText,
  isVisible,
  onClick,
  direction,
  Icon,
}: {
  tooltipText: string;
  isVisible: boolean;
  onClick: () => void;
  direction: 'left' | 'right';
  Icon: any;
}) => {
  const placementDirection = direction === 'left' ? 'right' : 'left';
  const isShowingLeft =
    (isVisible && direction === 'left') ||
    (!isVisible && direction === 'right');
  const ButtonComponent = direction === 'left' ? LeftButton : RightButton;
  return (
    <Tooltip
      title={<span style={{ fontSize: 14 }}>{tooltipText}</span>}
      placement={placementDirection}
      arrow
    >
      <ButtonComponent onClick={onClick}>
        <Icon style={{ fontSize: 18, marginRight: 3 }} />
        {isShowingLeft ? (
          <TabRightIcon style={{ fontSize: 14 }} />
        ) : (
          <TabLeftIcon style={{ fontSize: 14 }} />
        )}
      </ButtonComponent>
    </Tooltip>
  );
};

/* ------------------------------ GraphWidgets ------------------------------ */

export default function GraphWidgets() {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector(
    (state: RootState) =>
      state.cbdiEdit.cbdiEditCurrent.present.project.current,
  );
  const selectedTreeNodeConcept = useSelector(
    (state: RootState) =>
      state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept,
  );
  /* ----------------------------- useSates hooks ----------------------------- */
  const [noteEditViewIsVisible, setNoteEditViewIsVisible] =
    React.useState<boolean>(false);
  const [conceptMapIsVisible, setConceptMapIsVisible] =
    React.useState<boolean>(false);

  /* -------------------------------- Callbacks ------------------------------- */

  const onToggleNoteEditViewClicked = () => {
    setNoteEditViewIsVisible(!noteEditViewIsVisible);
  };
  const onToggleConceptMapClicked = () => {
    setConceptMapIsVisible(!conceptMapIsVisible);
  };

  /* -------------------------------- useMemo hooks ------------------------------- */

  const object = React.useMemo(() => {
    const mobject: CBDIEditorObject | undefined = getObjectByModuleConcept(
      current,
      selectedTreeNodeConcept,
    );
    return mobject;
  }, [selectedTreeNodeConcept, current]);

  return (
    <Root>
      <SubContainer className="leftSubContainer">
        {/* edit notes */}
        {/* only display when object is not undefined */}
        {object ? (
          <WidgetRoot
            isexpanded={noteEditViewIsVisible ? 'true' : 'false'}
            widgetwidth={344}
            widgetheight={180}
          >
            <WidgetContainer>
              <NotesEditView />
              <TooltipIconComponent
                tooltipText="Edit note"
                isVisible={noteEditViewIsVisible}
                onClick={onToggleNoteEditViewClicked}
                direction="left"
                Icon={NoteAltIcon}
              />
            </WidgetContainer>
          </WidgetRoot>
        ) : null}
      </SubContainer>
    </Root>
  );
}

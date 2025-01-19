/* eslint-disable react/function-component-definition */
import { styled } from '@mui/material';
import { nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import { CBDIEditorRootConceptType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import React from 'react';

/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  display: 'flex',
  flexDirection: 'row',
  borderRadius: 10,
  overflow: 'hidden',
  minWidth: 120,
});

const StyledToggleButton = styled('button')({
  padding: '0 15px',
  borderRadius: 0,
  fontSize: 14,
  '&.active': {
    backgroundColor: '#068cfa',
    color: '#ffffff',
    cursor: 'default',
  },
  '&.notActive': {
    '&:hover': {
      backgroundColor: '#56b6ff',
      color: '#ffffff',
    },
  },
});

interface ToggleButtonProps {
  isActive: boolean;
  image: string;
  text: string;
  onClick: () => void;
}

const ToggleButton: React.FC<ToggleButtonProps> = ({ isActive, image, text, onClick }) => {
  const handleClick = () => {
    onClick();
  };

  return (
    <StyledToggleButton disabled={isActive} className={isActive ? 'active' : 'notActive'} type="button" onClick={handleClick}>
      <img src={image} alt="" style={{ width: 30, height: 30 }} />
      <span className="button-text">{text}</span>
    </StyledToggleButton>
  );
};

interface ToggleTeamAgentButtonProps {
  value: 'team' | 'agent';
  onButtonClick: () => void;
}

const ToggleTeamAgentButton: React.FC<ToggleTeamAgentButtonProps> = ({ value, onButtonClick }) => (
  <Root>
    <ToggleButton
      isActive={value === CBDIEditorRootConceptType.TeamConceptType}
      image={nodeIcon[CBDIEditorRootConceptType.TeamConceptType]}
      text={CBDIEditorRootConceptType.TeamConceptType}
      onClick={() => onButtonClick()}
    />
    <ToggleButton
      isActive={value === CBDIEditorRootConceptType.AgentConceptType}
      image={nodeIcon[CBDIEditorRootConceptType.AgentConceptType]}
      text={CBDIEditorRootConceptType.AgentConceptType}
      onClick={() => onButtonClick()}
    />
  </Root>
);

export default ToggleTeamAgentButton;

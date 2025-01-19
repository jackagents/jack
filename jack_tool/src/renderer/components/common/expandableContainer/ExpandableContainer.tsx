import React, { useState, ReactNode, CSSProperties } from 'react';
import { styled } from '@mui/material';
import { ArrowBack, ArrowForward } from '@mui/icons-material';

/* --------------------------------- Styles --------------------------------- */
const ToggleButton = styled('div')({
  width: 30,
  height: 40,
  position: 'absolute',
  top: -1,
  right: 0,
  transform: 'translateX(100%)',
  borderRadius: '0 5% 5% 0',
  border: '1px solid black',
  backgroundColor: '#ededed',
  display: 'flex',
  alignItems: 'center',
  cursor: 'pointer',
});

interface ContainerProps {
  expanded: boolean;
  customStyles?: CSSProperties;
}

const Container = styled('div')<ContainerProps>(
  ({ expanded, customStyles }) => ({
    position: 'absolute',
    top: 0,
    left: 0,
    transform: expanded ? undefined : 'translateX(-100%)',
    border: '1px solid black',
    display: 'flex',
    flexDirection: 'column',
    transition: 'all 0.5s ease',
    backgroundColor: '#ffffff',
    ...customStyles, // Apply custom styles
  })
);

interface ExpandableContainerProps {
  children: ReactNode;
  customStyles?: CSSProperties;
  buttonTitle?: string;
}

const ExpandableContainer: React.FC<ExpandableContainerProps> = ({
  children,
  customStyles,
  buttonTitle,
}) => {
  const [expanded, setExpanded] = useState(false);

  const handleToggleExpand = () => {
    setExpanded((prev) => !prev);
  };

  return (
    <Container expanded={expanded} customStyles={customStyles}>
      <ToggleButton title={buttonTitle} onClick={handleToggleExpand}>
        {expanded ? <ArrowBack /> : <ArrowForward />}
      </ToggleButton>
      {children}
    </Container>
  );
};

export default ExpandableContainer;

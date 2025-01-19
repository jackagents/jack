import { Grid, Button, styled } from '@mui/material';
import { CREATOR_MENU } from 'constant/common/cmConstants';
import React from 'react';
import { GridContainer, StyledCustomList, ContentBox } from 'components/common/base/BaseContainer';
import FunctionLabel from 'components/common/functionLabel/FunctionLabel';

const StyledBtnContainerGrid = styled(Grid)({
  justifyContent: 'center',
  margin: '10px 0 10px',
});

const StyledBtn = styled(Button)(({}) => ({
  width: '8em',
}));

interface Props {
  hideClose?: boolean;
  onClose?: () => void;
  configDisplay: JSX.Element;
}

export default function CommonConfigEntityMenu({ hideClose = false, onClose, configDisplay }: Props) {
  const divRef = React.useRef<any>(null);

  return (
    <GridContainer ref={divRef} container sx={{ zIndex: 1000 }}>
      <StyledCustomList>
        <ContentBox>
          <FunctionLabel text={CREATOR_MENU.CONFIG} />
          {configDisplay}
        </ContentBox>

        <StyledBtnContainerGrid hidden={hideClose} container>
          <StyledBtn variant="contained" color="error" onClick={onClose}>
            Close
          </StyledBtn>
        </StyledBtnContainerGrid>
      </StyledCustomList>
    </GridContainer>
  );
}

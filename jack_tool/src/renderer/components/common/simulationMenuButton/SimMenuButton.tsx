import { Button, Grid, styled, Typography } from '@mui/material';
import {
  OverridableComponent,
  OverridableTypeMap,
} from '@mui/material/OverridableComponent';
import React from 'react';

const StyledButtonContainerGrid = styled(Grid)(() => ({
  alignContent: 'center',
  flexDirection: 'row',
}));

const StyledIconBtn = styled(Button)(({ theme }) => ({
  width: '100%',
  height: 'auto',
  margin: '10px 20px 10px 20px',
  justifyContent: 'flex-start',
  '&:hover': {
    backgroundColor: theme.custom?.simMenuBtn.bgColor,
  },
}));

const LabelGrid = styled(Grid)({
  marginLeft: '10px',
  alignContent: 'center',
  display: 'flex',
  alignItems: 'center',
});

const StyledLabelTypo = styled(Typography)(({ theme }) => ({
  width: 'auto',
  alignItems: 'center',
  color: theme.custom?.text.color,
}));

interface Props {
  id: number;
  icon: OverridableComponent<OverridableTypeMap>;
  text: string;
  onClick: (event: React.MouseEvent<HTMLElement>) => void;
  elements?: JSX.Element[];
}

/**
 * Common simulaton Menu Button
 * @param param0
 * @returns
 */
function SimMenuButton({ id, icon, text, onClick, elements = [] }: Props) {
  // Create custom icon with theme
  const CustomIcon = React.useMemo(() => {
    return styled(icon)(({ theme }) => ({
      color: theme.custom?.text.color,
    }));
  }, [icon]);

  return (
    <StyledButtonContainerGrid container>
      <StyledIconBtn id={id.toString()} onClick={onClick}>
        <CustomIcon fontSize="large" />

        <LabelGrid>
          <StyledLabelTypo>{text}</StyledLabelTypo>
        </LabelGrid>
      </StyledIconBtn>

      {...elements}
    </StyledButtonContainerGrid>
  );
}

export default React.memo(SimMenuButton);

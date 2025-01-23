import { Typography, Grid, styled } from '@mui/material';

interface Props {
  text: string;
}

const LabelGrid = styled(Grid)(({ theme }) => ({
  justifyContent: 'center',
  height: theme.custom?.grid.height.medium,
}));

const FunctionLabelTypo = styled(Typography)(({ theme }) => ({
  position: 'relative',
  top: '-1vw',
  backgroundColor: theme.custom?.backgroundColor,
  color: theme.custom?.text.color,
  width: 'fit-content',
  padding: '0 5px 0 5px',
  fontSize: theme.custom?.text.size.large,
}));

export default function FunctionLabel(props: Props) {
  const { text } = props;
  return (
    <LabelGrid container>
      <FunctionLabelTypo variant="h6" align="center" top="-15px">
        {text}
      </FunctionLabelTypo>
    </LabelGrid>
  );
}

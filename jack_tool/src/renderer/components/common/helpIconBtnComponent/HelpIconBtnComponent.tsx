import { styled, Tooltip, TooltipProps, tooltipClasses } from '@mui/material';
import HelpOutlinedIcon from '@mui/icons-material/HelpOutlined';

const DarkToolTip = styled(({ className, ...props }: TooltipProps) => (
  // eslint-disable-next-line react/jsx-props-no-spreading
  <Tooltip {...props} classes={{ popper: className }} />
))(({ theme }) => ({
  [`& .${tooltipClasses.tooltip}`]: {
    backgroundColor: 'black',
    color: 'white',
    border: '1px solid white',
    fontSize: '1.4vh',
  },
}));

const HelpIcon = styled(HelpOutlinedIcon)(({ theme }) => ({
  marginLeft: '5px',
  width: '1rem',
  height: '1rem',
  '&:hover': { color: theme.sceanario.aewcf.highlight },
}));

interface Props {
  content: string;
}

export default function HelpIconBtnComponent({ content }: Props) {
  return (
    <DarkToolTip disableFocusListener disableTouchListener TransitionProps={{ timeout: 0 }} placement="right-end" title={content}>
      <HelpIcon />
    </DarkToolTip>
  );
}

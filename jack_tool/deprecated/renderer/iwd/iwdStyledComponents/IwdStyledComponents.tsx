import {
  ButtonGroup,
  IconButton,
  styled,
  Tooltip,
  tooltipClasses,
  TooltipProps,
} from '@mui/material';

/**
 * Styled Icon button used in vehicle control
 */
export const VehicleControlIconBtn = styled(IconButton)({
  borderRadius: '0px',
  ':hover': {
    border: `1px solid aqua`,
  },
});

/**
 * Styled Tooltip used in vehicle control
 */
export const VehicleControlTooltip = styled(
  ({ className, ...props }: TooltipProps) => (
    // eslint-disable-next-line react/jsx-props-no-spreading
    <Tooltip {...props} classes={{ popper: className }} />
  )
)({
  [`& .${tooltipClasses.tooltip}`]: {
    top: '-10px',
  },
});

/**
 * Styled button group used in vehicle control
 */
export const IconButtonGroup = styled(ButtonGroup)({
  alignItems: 'center',
});

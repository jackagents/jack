import {
  VehicleControlIconBtn,
  VehicleControlTooltip,
} from 'components/iwd/iwdStyledComponents/IwdStyledComponents';
import {
  styled,
  SvgIconTypeMap,
  SxProps,
  Theme,
  Typography,
} from '@mui/material';
import { OverridableComponent } from '@mui/material/OverridableComponent';
import { IwdIconButtonId } from 'types/iwd/iwdTypes';

interface Props {
  /**
   * button id
   */
  id: IwdIconButtonId;
  /**
   * on click callback
   */
  onClick: (event: React.MouseEvent<HTMLButtonElement>) => void;
  /**
   * button label
   */
  label: string;
  /**
   * button icon
   */
  icon: OverridableComponent<SvgIconTypeMap<{}, 'svg'>>;
  /**
   * optional style for icon
   */
  optionalStyle?: SxProps<Theme>;
  /**
   * disable state
   */
  disabled?: boolean;
  /**
   * activated state
   */
  activated?: boolean;
}

export default function TemplateIconButton({
  id,
  label,
  icon,
  onClick,
  optionalStyle,
  disabled = false,
  activated = false,
}: Props) {
  const StyledIcon = styled(icon)(({ theme }) => ({
    color: activated ? theme.iwd?.button.activated : theme.custom?.text.color,
    fontSize: theme.custom?.text.size.large,
  }));

  const DisabledIcon = styled(icon)(({ theme }) => ({
    color: '#5e5c5c',
    fontSize: theme.custom?.text.size.large,
  }));

  return (
    <VehicleControlTooltip
      title={<Typography>{label}</Typography>}
      arrow
      placement="bottom-start"
    >
      <span>
        <VehicleControlIconBtn disabled={disabled} id={id} onClick={onClick}>
          {disabled ? (
            <DisabledIcon sx={optionalStyle} />
          ) : (
            <StyledIcon sx={optionalStyle} />
          )}
        </VehicleControlIconBtn>
      </span>
    </VehicleControlTooltip>
  );
}

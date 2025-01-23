import { MenuItem, outlinedInputClasses, Select, SelectChangeEvent, styled, SxProps, Theme } from '@mui/material';
import { PropsWithChildren } from 'react';

interface BasicSelectComponentProps extends PropsWithChildren {
  value: any;
  onChange: (event: SelectChangeEvent<unknown>) => void;
  onClose?: () => void;
  onOpen?: () => void;
  hasNone?: boolean;
  style?: SxProps<Theme>;
  disabled?: boolean;
  noneItemStyle?: React.CSSProperties;
}

const StyledSelectEventProp = styled(Select)({
  overflow: 'hidden',
  width: '100%',
  height: '30px',
  // on focus
  [`&.${outlinedInputClasses.focused} .${outlinedInputClasses.notchedOutline}`]: {
    borderColor: 'rgba(0,0,0,0.5)',
  },
});

export default function BasicSelectComponent({
  children,
  value,
  hasNone = true,
  disabled = false,
  style,
  noneItemStyle,
  onChange,
  onClose,
  onOpen,
}: BasicSelectComponentProps) {
  return (
    <StyledSelectEventProp
      sx={{ height: '100%', ...style }}
      value={value}
      disabled={disabled}
      onChange={onChange}
      onOpen={onOpen}
      onClose={onClose}
      MenuProps={{
        transitionDuration: 0,
      }}
    >
      {hasNone && (
        <MenuItem style={{ ...noneItemStyle }} value="">
          <em>None</em>
        </MenuItem>
      )}

      {children}
    </StyledSelectEventProp>
  );
}

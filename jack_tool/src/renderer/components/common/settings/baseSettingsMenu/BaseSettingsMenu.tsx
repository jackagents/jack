import { styled, Typography, Stack } from '@mui/material';
import { Fluid } from 'components/common/base/BaseContainer';
import { PropsWithChildren } from 'react';
import ThemeSelector from '../themeSelector/ThemeSelector';

const StyledFluid = styled(Fluid)(({ theme }) => ({
  paddingTop: 50,
  paddingLeft: 50,
  backgroundColor: theme.custom?.backgroundColor,
  color: theme.custom?.text.color,
}));

const StyledText = styled(Typography)(({ theme }) => ({
  color: theme.custom?.text.color,
}));

export const SettingItemStyledStack = styled(Stack)(({ theme }) => ({
  marginTop: '30px',
  paddingRight: '50px',
  alignItems: 'center',
  color: theme.custom?.text.color,
}));

type Props = PropsWithChildren;

/**
 * Base setting page with theme selector
 * @param param0
 * @returns
 */
export default function BaseSettingsMenu({ children }: Props) {
  return (
    <StyledFluid>
      <StyledText variant="h4">Setting</StyledText>

      <SettingItemStyledStack direction="row">
        <Stack style={{ width: '200px' }}>
          <Typography>Theme</Typography>
        </Stack>

        <ThemeSelector />
      </SettingItemStyledStack>
      {children}
    </StyledFluid>
  );
}

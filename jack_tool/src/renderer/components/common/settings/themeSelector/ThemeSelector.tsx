import React from 'react';
import { MenuItem, SelectChangeEvent, Stack, useTheme } from '@mui/material';
import { useCustomThemeContext } from 'components/common/customThemeProvider/CustomThemeProvider';
import { ThemeType } from 'misc/types/common/cmTypes';
import BasicSelectComponent from 'components/common/basicSelectComponent/BasicSelectComponent';

export default function ThemeSelector() {
  const { themeName, setThemeName } = useCustomThemeContext();
  const theme = useTheme();
  const [value, setValue] = React.useState<ThemeType>(themeName);

  const handleChange = (event: SelectChangeEvent<unknown>) => {
    setValue(event.target.value as ThemeType);
  };

  React.useEffect(() => {
    setThemeName(value);
  }, [value]);

  return (
    <Stack
      style={{
        width: 'auto',
        minWidth: '300px',
        justifyContent: 'flex-start',
      }}
    >
      <BasicSelectComponent
        style={{
          '& .MuiSvgIcon-root': {
            color: theme.editor.settingPage.textColor,
          },
          '.MuiSelect-outlined': {
            borderColor: theme.editor.settingPage.textColor,
            color: theme.editor.settingPage.textColor,
          },
          '.MuiOutlinedInput-notchedOutline': {
            borderColor: theme.editor.settingPage.textColor,
          },
          '&.Mui-focused .MuiOutlinedInput-notchedOutline': {
            borderColor: theme.editor.settingPage.textColor,
          },
          '&:hover .MuiOutlinedInput-notchedOutline': {
            borderColor: theme.sceanario.aewcf.highlight,
          },
        }}
        value={value}
        onChange={handleChange}
        hasNone={false}
      >
        <MenuItem key="Dark" value="Dark">
          <span>Dark</span>
        </MenuItem>

        <MenuItem key="Light" value="Light">
          <span>Light</span>
        </MenuItem>
      </BasicSelectComponent>
    </Stack>
  );
}

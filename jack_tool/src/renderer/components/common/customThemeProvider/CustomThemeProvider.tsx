import { ThemeProvider } from '@mui/material';
import { darkTheme, lightTheme } from 'misc/theme/Theme';
import { ThemeType } from 'misc/types/common/cmTypes';
import { getValueFromLocalStorage, storeValueToLocalStorage } from 'misc/utils/browser/localStorage/LocalStorageUtils';
import React, { PropsWithChildren } from 'react';

export const CustomThemeContext = React.createContext<{
  themeName: ThemeType;
  setThemeName: React.Dispatch<React.SetStateAction<ThemeType>>;
}>({
  themeName: 'Dark',
  setThemeName: () => {},
});

/**
 * Custom theme provider
 * @param param0
 * @returns
 */
export default function CustomThemeProvider({ children }: PropsWithChildren) {
  const [theme, setTheme] = React.useState(darkTheme);
  const [themeName, setThemeName] = React.useState<ThemeType>((getValueFromLocalStorage('themeName') as ThemeType) || 'Dark');

  React.useEffect(() => {
    if (themeName === 'Dark') {
      setTheme(darkTheme);
    } else if (themeName === 'Light') {
      setTheme(lightTheme);
    }

    storeValueToLocalStorage('themeName', themeName);
  }, [themeName]);

  return (
    <CustomThemeContext.Provider value={{ themeName, setThemeName }}>
      <ThemeProvider theme={theme}>{children}</ThemeProvider>
    </CustomThemeContext.Provider>
  );
}

export const useCustomThemeContext = () => React.useContext(CustomThemeContext);

import { Theme } from '@mui/material';
import React from 'react';
import { darkTheme, lightTheme } from 'theme';
import {
  Container,
  CustomFormControlLabel,
  CustomFormGroup,
  CustomSwitch,
} from '../base/BaseContainer';

interface Props {
  setAppTheme: (theme: Theme) => void;
}

export default function ThemeToggle({ setAppTheme }: Props) {
  const [dark, setDark] = React.useState(true);

  React.useEffect(() => {
    if (dark) {
      setAppTheme(darkTheme);
    } else {
      setAppTheme(lightTheme);
    }
  }, [dark]);

  return (
    <Container container>
      <CustomFormGroup>
        <CustomFormControlLabel
          control={
            <CustomSwitch
              onChange={() => {
                setDark(!dark);
              }}
              defaultChecked={false}
            />
          }
          label={dark ? 'Dark' : 'Light'}
        />
      </CustomFormGroup>
    </Container>
  );
}

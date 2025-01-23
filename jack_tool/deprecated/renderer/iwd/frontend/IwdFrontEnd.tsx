import React from 'react';
import { ThemeProvider } from '@mui/material';
import {
  ClientRow,
  MenubarRow,
  Root,
  ToolbarRow,
} from 'components/common/base/BaseContainer';
import IwdMainInterface from 'components/iwd/interface/IwdMainInterface';
import { darkTheme } from 'misc/theme/Theme';
import Toolbar from 'components/common/toolbar/Toolbar';
import TitleMenu from 'components/common/titleMenu/TitleMenu';
import cmAppMenuTemplate from 'misc/menuTemplate/common/cmMenuTemplate';
import ThemeToggle from 'components/common/themeToggle/ThemeToggle';
import LiveWatch from 'components/common/liveWatch/LiveWatch';
import { useSliceSelector } from 'projectRedux/sliceProvider/SliceProvider';
import { IwdClientState } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { MODE } from 'constant/iwd/iwdConstant';
import ClientStatusControl from 'components/iwd/clientStatusControl/ClientStatusControl';

/* ************************************************************************************************
 * Client
 * *********************************************************************************************** */

function FrontEnd() {
  // default theme is dark
  const [theme, setTheme] = React.useState(darkTheme);

  const { explorer } = useSliceSelector() as IwdClientState;

  const toolBarElements = React.useMemo(() => {
    // Only render the toolbar when not in explainability mode
    if (explorer.mode !== MODE.MODE_RUNTIME_EXPLAINABILITY) {
      return [
        <ThemeToggle key="theme-toggle" setAppTheme={setTheme} />,
        <ClientStatusControl key="client-status-contrl" />,
        <LiveWatch key="live-watch" />,
      ];
    }

    return [
      <ThemeToggle key="theme-toggle" setAppTheme={setTheme} />,
      <LiveWatch key="live-watch" />,
    ];
  }, [explorer.mode]);

  return (
    <ThemeProvider theme={theme}>
      <Root>
        <MenubarRow>
          <TitleMenu menuAction={cmAppMenuTemplate} />
        </MenubarRow>

        <ToolbarRow>
          <Toolbar elements={toolBarElements} />
        </ToolbarRow>

        <ClientRow>
          <IwdMainInterface />
        </ClientRow>
      </Root>
    </ThemeProvider>
  );
}

/* ************************************************************************************************
 * Connect to Redux Store
 * *********************************************************************************************** */
export default React.memo(FrontEnd);

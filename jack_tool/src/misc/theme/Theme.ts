/* eslint-disable @typescript-eslint/no-empty-interface */
import { createTheme } from '@mui/material';

declare module '@mui/material/styles' {
  /**
   * Custom type for typescript
   */
  interface CustomTheme {
    theme: 'dark' | 'light';
    aosColor: '#00a1b1';
    custom?: {
      headerBgColor: string;
      backgroundColor: string;
      toolbarBgColor: string;
      dividerBgColor: string;
      closeBtnHoveredColor: string;
      setting: {
        textColor: string;
      };
      body: {
        bgColor: string;
      };
      sortIcon: {
        bgColor: string;
      };
      tab: {
        selectedTab: {
          bgColor: string;
        };
      };
      sideMenu: {
        bgColor: string;
        iconColor: string;
        checkedIconColor: string;
      };
      switch: {
        switchBaseCheckedColor: string;
        switchBaseCheckedTrackBgColor: string;
        switchDisabledColor: string;
        switchTrackBgColor: string;
      };
      border: {
        thin: string;
        left: string;
        color: {
          primary?: string;
          secondary?: string;
          warn?: string;
        };
      };
      text: {
        color: string;
        size: {
          small?: string;
          medium?: string;
          large?: string;
          vHSmall?: string;
        };
      };
      grid: {
        height: {
          small?: string;
          medium?: string;
          large?: string;
        };
      };
      simMenuBtn: {
        bgColor: string;
      };
      list: {
        size: { height?: string; width?: string };
      };
      item: {
        clicked: {
          bgColor: string;
          hoveredBgColor: string;
        };
        normal: {
          hoveredBgColor: string;
        };
      };
      button: {
        bgColor: {
          positive: string;
          negative: string;
        };
        textColor: string;
      };
      checkBox: {
        bgColor: string;
      };
    };
    iba: {
      menu: {
        textColor: string;
        activeTextColor: string;
        bgColor: string;
        hoveringBgColor: string;
      };
    };
    iwd?: {
      button: {
        activated: string;
      };
    };
    editor: {
      form: {
        bgColor: {
          preset1: string;
        };
      };
      button: {
        fontWeight: string;
        preset1: {
          bgColor: string;
          textColor: string;
          hoveredBgColor: string;
          hoveredTextColor: string;
        };
        preset2?: {
          bgColor: string;
          textColor: string;
        };
        preset3?: {
          bgColor: string;
          textColor: string;
        };
        preset4?: {
          bgColor: string;
          textColor: string;
        };
        preset5?: {
          bgColor: string;
          textColor: string;
        };
        preset6?: {
          bgColor: string;
          textColor: string;
        };
      };
      menu: {
        textColor: string;
        activeTextColor: string;
        bgColor: string;
        hoveringBgColor: string;
      };
      welcomePage: {
        textColor: string;
        linkColor: string;
        bgColor: string;
        hoveringBgColor: string;
      };
      settingPage: {
        textColor: string;
        bgColor: string;
      };
      explorerView: {
        textColor: string;
        contrastTextColor: string;
        bgColor: string;
        hoveringBgColor: string;
        activeBgColor: string;
      };
      consoleView: {
        textColor: string;
        bgColor: string;
        hoveringBgColor: string;
        activeBgColor: string;
      };
      graphView: {
        textColor: string;
        bgColor: string;
        gridLinearColor: string;
        graphLineColor: string;
        graphNodeColor: string;
      };
      detailView: {
        textColor: string;
        bgColor: string;
        disableColor: string;
      };
      widget: {
        textColor: string;
        bgColor: string;
      };
      universalSearch: {
        textColor: string;
        hoveringBgColor: string;
        bgColor: string;
      };
      select: {
        textColor: string;
        emptyTextColor: string;
        menuItemTextColor: string;
        menuItemActiveTextColor: string;
        menuItemActiveBgColor: string;
        menuItemBgColor: string;
        menuItemBorderColor: string;
        disableColor: string;
      };
      textInput: {
        textColor: string;
        errorTextColor: string;
        disableColor: string;
      };
      scrollBar: {
        thumbBgColor: string;
        thumbHoverBgColor: string;
      };
    };
    sceanario: {
      aewcf: {
        highlight: string;
        path: string;
      };
    };
  }

  interface Theme extends CustomTheme {}
  interface ThemeOptions extends CustomTheme {}
}

export const lightTheme = createTheme({
  /**
   * Disable ripple globally
   */
  // components: {
  //   MuiButton: {
  //     defaultProps: { disableRipple: true },
  //   },
  //   MuiMenuItem: {
  //     defaultProps: { disableRipple: true },
  //   },
  // },
  theme: 'light',
  aosColor: '#00a1b1',
  sceanario: {
    aewcf: {
      highlight: 'blue',
      path: 'black',
    },
  },
  custom: {
    headerBgColor: '#e5e5e5',
    backgroundColor: '#ffffff',
    toolbarBgColor: '#e8e8e8',
    dividerBgColor: '#fc9fc1',
    closeBtnHoveredColor: '#e02a2a',
    setting: {
      textColor: 'black',
    },
    body: {
      bgColor: 'whitesmoke',
    },
    sortIcon: {
      bgColor: '#e0dede',
    },
    sideMenu: {
      bgColor: '#d6cfe3',
      iconColor: '#9369e0',
      checkedIconColor: 'black',
    },
    tab: {
      selectedTab: {
        bgColor: '#f3d5f5',
      },
    },
    switch: {
      switchBaseCheckedColor: '#f50057',
      switchBaseCheckedTrackBgColor: '#d81b60',
      switchDisabledColor: '#ffffff',
      switchTrackBgColor: 'grey',
    },
    border: {
      thin: '1px solid black',
      left: '2px solid black',
      color: {
        primary: 'black',
        secondary: 'blue',
        warn: 'teal',
      },
    },
    text: {
      color: '#000000',
      size: {
        small: '0.8vw',
        medium: '1vw',
        large: '1.3vw',
        vHSmall: '1.5vh',
      },
    },
    grid: {
      height: {
        medium: '1.2vw',
      },
    },
    simMenuBtn: {
      bgColor: 'green',
    },
    list: {
      size: {
        height: '3.5vw',
      },
    },
    item: {
      clicked: {
        bgColor: 'green',
        hoveredBgColor: 'green',
      },
      normal: {
        hoveredBgColor: '#aaadab',
      },
    },
    button: {
      bgColor: {
        positive: '#2e7d32',
        negative: '#d32f2f',
      },
      textColor: 'black',
    },
    checkBox: {
      bgColor: 'grey',
    },
  },
  iba: {
    menu: {
      textColor: '#8e8e8f',
      activeTextColor: '#f0f0f0',
      bgColor: '#2c2c2c',
      hoveringBgColor: '#2a2d2e',
    },
  },
  iwd: {
    button: {
      activated: 'green',
    },
  },
  editor: {
    form: {
      bgColor: {
        preset1: '#CCCCCC',
      },
    },
    button: {
      fontWeight: 'bold',
      preset1: {
        bgColor: '#e09e58',
        textColor: 'black',
        hoveredBgColor: 'green',
        hoveredTextColor: 'white',
      },
      preset2: {
        bgColor: '#2a8c62',
        textColor: 'black',
      },
      preset3: {
        bgColor: '#ffffff30',
        textColor: 'black',
      },
      preset4: {
        bgColor: '#ffffff90',
        textColor: 'black',
      },
      preset5: {
        bgColor: '#0747A6',
        textColor: '#ffffff',
      },
      preset6: {
        bgColor: '#4C9AFF',
        textColor: 'black',
      },
    },
    menu: {
      textColor: '#8e8e8f',
      activeTextColor: '#f0f0f0',
      bgColor: '#2c2c2c',
      hoveringBgColor: '#2a2d2e',
    },
    welcomePage: {
      textColor: '#000000',
      linkColor: '#2160bb',
      bgColor: '#dddddd',
      hoveringBgColor: '#ffffff90',
    },
    settingPage: {
      textColor: '#000000',
      bgColor: '#dddddd',
    },
    explorerView: {
      textColor: '#000000',
      contrastTextColor: '#ffffff',
      bgColor: '#dddddd',
      hoveringBgColor: '#ffffff50',
      activeBgColor: '#2d92f1',
    },
    consoleView: {
      textColor: '#000000',
      bgColor: '#dddddd',
      hoveringBgColor: '#ffffff90',
      activeBgColor: '#068cfa96',
    },
    graphView: {
      textColor: '#000000',
      bgColor: '#ffffff',
      gridLinearColor: '#dddddd',
      graphLineColor: '#000000',
      graphNodeColor: '#000000',
    },
    detailView: {
      textColor: '#000000',
      bgColor: '#dddddd',
      disableColor: 'grey',
    },
    widget: {
      textColor: '#000000',
      bgColor: '#dddddd',
    },
    universalSearch: {
      textColor: '#000000',
      bgColor: '#dddddd',
      hoveringBgColor: '#cccccc',
    },
    select: {
      textColor: '#000000',
      menuItemActiveTextColor: '#ffffff',
      emptyTextColor: '#555555',
      menuItemTextColor: '#000000',
      menuItemBgColor: '#ffffff',
      menuItemActiveBgColor: '#2d92f1',
      menuItemBorderColor: '#d4d4d4',
      disableColor: 'grey',
    },
    textInput: {
      textColor: 'black',
      errorTextColor: 'red',
      disableColor: 'grey',
    },
    scrollBar: {
      thumbBgColor: '#00000030',
      thumbHoverBgColor: '#00000050',
    },
  },
});

export const darkTheme = createTheme({
  /**
   * Disable ripple globally
   */
  // components: {
  //   MuiButton: {
  //     defaultProps: { disableRipple: true },
  //   },
  //   MuiMenuItem: {
  //     defaultProps: { disableRipple: true },
  //   },
  // },
  theme: 'dark',
  aosColor: '#00a1b1',
  sceanario: {
    aewcf: {
      highlight: 'blue',
      path: 'black',
    },
  },
  custom: {
    headerBgColor: '#404040',
    backgroundColor: '#333333',
    toolbarBgColor: '#5d5d5d',
    dividerBgColor: '#fc9fc1',
    closeBtnHoveredColor: '#e02a2a',
    setting: {
      textColor: 'black',
    },
    body: {
      bgColor: '#c4c4c4',
    },
    sortIcon: {
      bgColor: 'grey',
    },
    sideMenu: {
      bgColor: '#2c2c2c',
      iconColor: '#f50057',
      checkedIconColor: 'white',
    },
    tab: {
      selectedTab: {
        bgColor: '#555555',
      },
    },
    switch: {
      switchBaseCheckedColor: '#f50057',
      switchBaseCheckedTrackBgColor: '#d81b60',
      switchDisabledColor: '#ffffff',
      switchTrackBgColor: 'grey',
    },
    border: {
      thin: '1px solid white',
      color: {
        primary: 'white',
        secondary: 'green',
        warn: 'yellow',
      },
      left: '2px solid white',
    },
    text: {
      color: '#ffffff',
      size: {
        small: '0.8vw',
        medium: '1vw',
        large: '1.3vw',
        vHSmall: '1.5vh',
      },
    },
    grid: {
      height: {
        medium: '1.2vw',
      },
    },
    simMenuBtn: {
      bgColor: 'green',
    },
    list: {
      size: {
        height: '3.5vw',
      },
    },
    item: {
      clicked: {
        bgColor: 'green',
        hoveredBgColor: 'green',
      },
      normal: {
        hoveredBgColor: '#aaadab',
      },
    },
    button: {
      bgColor: {
        positive: 'green',
        negative: 'red',
      },
      textColor: 'white',
    },
    checkBox: {
      bgColor: 'grey',
    },
  },
  iba: {
    menu: {
      textColor: '#8e8e8f',
      activeTextColor: '#f0f0f0',
      bgColor: '#333333',
      hoveringBgColor: '#2a2d2e',
    },
  },
  iwd: {
    button: {
      activated: 'lime',
    },
  },
  editor: {
    form: {
      bgColor: {
        preset1: '#CCCCCC',
      },
    },
    button: {
      fontWeight: 'bold',
      preset1: {
        bgColor: '#e09e58',
        textColor: 'black',
        hoveredBgColor: 'green',
        hoveredTextColor: 'white',
      },
      preset2: {
        bgColor: '#2a8c62',
        textColor: 'black',
      },
      preset3: {
        bgColor: '#ffffff30',
        textColor: 'black',
      },
      preset4: {
        bgColor: '#ffffff90',
        textColor: 'black',
      },
      preset5: {
        bgColor: '#0747A6',
        textColor: '#ffffff',
      },
      preset6: {
        bgColor: '#4C9AFF',
        textColor: 'black',
      },
    },
    menu: {
      textColor: '#8e8e8f',
      activeTextColor: '#f0f0f0',
      bgColor: '#333333',
      hoveringBgColor: '#2a2d2e',
    },
    welcomePage: {
      textColor: '#cccccc',
      linkColor: '#3c93f9',
      bgColor: '#1e1e1e',
      hoveringBgColor: '#2a2d2e',
    },
    settingPage: {
      textColor: '#cccccc',
      bgColor: '#1e1e1e',
    },
    explorerView: {
      textColor: '#cccccc',
      contrastTextColor: '#ffffff',
      bgColor: '#2a2a2a',
      hoveringBgColor: '#2a2d2e',
      activeBgColor: '#37373d',
    },
    consoleView: {
      textColor: '#cccccc',
      bgColor: '#2a2a2a',
      hoveringBgColor: '#ffffff90',
      activeBgColor: '#068cfa96',
    },
    graphView: {
      textColor: '#ffffff',
      bgColor: '#2a2a2a',
      gridLinearColor: '#ffffff',
      graphLineColor: '#cccccc',
      graphNodeColor: '#cccccc',
    },
    detailView: {
      textColor: '#cccccc',
      bgColor: '#2a2a2a',
      disableColor: 'grey',
    },
    widget: {
      textColor: '#ffffff',
      bgColor: '#252526',
    },
    universalSearch: {
      textColor: '#ffffff',
      bgColor: '#252526',
      hoveringBgColor: '#cccccc',
    },
    select: {
      textColor: '#cccccc',
      menuItemActiveTextColor: '#ffffff',
      emptyTextColor: '#888888',
      menuItemTextColor: '#000000',
      menuItemBgColor: '#ffffff',
      menuItemActiveBgColor: '#2d92f1',
      menuItemBorderColor: '#454545',
      disableColor: 'grey',
    },
    textInput: {
      textColor: '#cccccc',
      errorTextColor: 'red',
      disableColor: 'grey',
    },
    scrollBar: {
      thumbBgColor: '#ffffff30',
      thumbHoverBgColor: '#ffffff50',
    },
  },
});

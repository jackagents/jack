import React from 'react';
import { useSelector } from 'react-redux';
import { styled } from '@mui/material';
import Map from 'components/iwd/map/iwdMap';
import { Fluid, Row, Col } from 'components/common/base/BaseContainer';
import SideMenu from 'components/iwd/sideMenu/SideMenu';
import SettingMenu from 'components/common/settings/SettingMenu';
import { MODE } from 'constant/iwd/iwdConstant';
import { RootState } from 'projectRedux/Store';
import SimMap from 'components/iwd/simMap/SimMap';
import RuntimeExplainability from 'components/common/runtimeExplainability/RuntimeExplainability';

/* ************************************************************************************************
 * Styles
 * *********************************************************************************************** */

const BodyRow = styled(Row)(({ theme }) => ({
  top: 0,
  bottom: 0,
  borderRight: `5px solid ${theme.custom?.sideMenu.bgColor}`,
}));

const SideMenuCol = styled(Col)({
  width: 50,
  left: 0,
  zIndex: 0,
});

const SplitViewCol = styled(Col)({
  left: 50,
  right: 0,
});

const StyledContainer = styled(Fluid)(({ theme }) => ({
  backgroundColor: theme.custom?.body.bgColor,
}));

function IwdMainInterface() {
  const { explorer } = useSelector((state: RootState) => {
    return state.iwd;
  });

  const fluids = React.useMemo(() => {
    if (explorer.mode === MODE.MODE_RUNTIME_EXPLAINABILITY) {
      return (
        <Fluid id="fluid-runtime-explainability">
          <RuntimeExplainability />
        </Fluid>
      );
    }

    return (
      <>
        <Fluid
          hidden={
            explorer.mode === MODE.MODE_SETTINGS ||
            explorer.mode === MODE.MODE_SIM_MAP
          }
        >
          <Map />
        </Fluid>

        <Fluid hidden={explorer.mode !== MODE.MODE_SETTINGS}>
          <SettingMenu />
        </Fluid>

        <Fluid id="fluid-sim-map" hidden={explorer.mode !== MODE.MODE_SIM_MAP}>
          <SimMap />
        </Fluid>
      </>
    );
  }, [explorer.mode]);

  return (
    <StyledContainer>
      <BodyRow>
        <SideMenuCol>
          <SideMenu />
        </SideMenuCol>
        <SplitViewCol id="content-viewport">{fluids}</SplitViewCol>
      </BodyRow>
    </StyledContainer>
  );
}

export default React.memo(IwdMainInterface);

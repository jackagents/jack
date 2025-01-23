import { ButtonGroup, styled } from '@mui/material';
import React from 'react';
import {
  PopupFluid,
  ThemeColorBtn,
} from 'components/common/base/BaseContainer';
import L from 'leaflet';

const StyledBtnGrp = styled(ButtonGroup)(({ theme }) => ({
  width: 170,
  margin: '15px 15px 30px 15px',
  backgroundColor: theme.custom?.backgroundColor,
  pointerEvents: 'stroke',
}));

interface Props {
  onClick: (event: React.MouseEvent<HTMLButtonElement>) => void;
  hideSave?: boolean;
  hideCancel?: boolean;
  elements?: JSX.Element[] | JSX.Element;
}

function Modal({
  hideSave = false,
  hideCancel = false,
  elements = [],
  onClick,
}: Props) {
  const divRef = React.useRef<HTMLDivElement>(null);

  const children = React.useMemo(() => {
    if (Object.prototype.toString.call(elements) === '[object Array]') {
      return (elements as JSX.Element[]).map((x) => x);
    }

    return elements;
  }, [elements]);

  React.useEffect(() => {
    if (divRef.current) L.DomEvent.disableClickPropagation(divRef.current);
  }, []);

  return (
    <PopupFluid>
      <StyledBtnGrp id="button-group" ref={divRef} orientation="vertical">
        {children}

        <ThemeColorBtn hidden={hideSave} id="save" onClick={onClick}>
          Save
        </ThemeColorBtn>

        <ThemeColorBtn hidden={hideCancel} id="cancel" onClick={onClick}>
          Cancel
        </ThemeColorBtn>
      </StyledBtnGrp>
    </PopupFluid>
  );
}

export default React.memo(Modal);

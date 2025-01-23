import { styled } from '@mui/material';
import React, { useMemo, useState } from 'react';
import ExpandMoreIcon from '@mui/icons-material/ExpandMore';
import ExpandLessIcon from '@mui/icons-material/ExpandLess';
import { getUniqueKeys } from './helper';

type NestedTableProps = {
  jsonData: Record<string, any> | any[];
};

const MARGIN_UNIT = 10;
const GAP_UNIT = 10;
const PADDING_UNIT = 10;
/* --------------------------------- Styles --------------------------------- */

const ControlButton = styled('button')({
  width: '40%',
  flex: 1,
  display: 'inline-block',
  padding: '8px 10px',
  fontSize: 12,
  fontWeight: 'bold',
  textTransform: 'uppercase',
  color: '#ffffff',
  backgroundColor: '#068cfa',
  border: 'none',
  borderRadius: 5,
  cursor: 'pointer',
  boxShadow: '0px 2px 5px rgba(0, 0, 0, 0.1)',
  '&:hover': {
    backgroundColor: '#1976d2',
  },
});

const Container = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  gap: GAP_UNIT,
  alignItems: 'flex-start',
  alignSelf: 'stretch',
  alignContent: 'flex-start',
  textAlign: 'left',
  border: '1px solid black',
  marginLeft: MARGIN_UNIT,
  marginRight: MARGIN_UNIT,
  padding: PADDING_UNIT,
  overflow: 'hidden',
  textOverflow: 'ellipsis',
});

const Header = styled('div')({
  alignSelf: 'stretch',
  display: 'flex',
  alignItems: 'center',
  justifyContent: 'space-between',
});

const Content = styled(Container)({
  '&:nth-of-type(odd)': {
    backgroundColor: '#f0f0f0', // background color for odd children
  },
  '&:nth-of-type(even)': {
    backgroundColor: 'transparent', // background color for even children
  },
});

function NestedTable({ jsonData }: NestedTableProps) {
  const [expanded, setExpanded] = useState<Record<string, boolean>>({});

  /* ------------------------------ useMemo hooks ----------------------------- */
  const allUniqueParentId = useMemo(() => getUniqueKeys(jsonData), [jsonData]);

  /* -------------------------------- Callbacks ------------------------------- */
  const toggleRow = (key: string) => {
    setExpanded({ ...expanded, [key]: !expanded[key] });
  };

  const toggleAllRow = () => {
    if (expanded.root) {
      setExpanded({});
    } else {
      const newExpanded: Record<string, boolean> = {};
      allUniqueParentId.forEach((id) => {
        newExpanded[id] = true;
      });
      setExpanded(newExpanded);
    }
  };

  /* -------------------------------- Functions ------------------------------- */
  const renderRows = (data: Record<string, any> | any[], isParentArray: boolean, parentKey = 'root', parentId = 'root') => {
    if (Array.isArray(data)) {
      return (
        <Container key={parentKey}>
          {!isParentArray && (
            <Header onClick={() => toggleRow(parentId)}>
              <div>{parentKey}</div>
              {expanded[parentId] ? <ExpandLessIcon /> : <ExpandMoreIcon />}
            </Header>
          )}
          {(isParentArray || expanded[parentId]) &&
            data.map((item, index) => {
              const newParentKey = index.toString();
              const newParentId = `${parentId}/${index}`;

              return renderRows(item, true, newParentKey, newParentId);
            })}
        </Container>
      );
    }

    if (typeof data === 'object') {
      return (
        <Container key={parentKey} style={{ border: isParentArray ? 'none' : undefined }}>
          {!isParentArray && (
            <Header onClick={() => toggleRow(parentId)}>
              <div>{parentKey}</div>
              {expanded[parentId] ? <ExpandLessIcon /> : <ExpandMoreIcon />}
            </Header>
          )}
          {(isParentArray || expanded[parentId]) &&
            Object.keys(data).map((key) => {
              const newParentKey = key;
              const newParentId = `${parentId}/${key}`;
              return renderRows(data[key], false, newParentKey, newParentId);
            })}
        </Container>
      );
    }
    return (
      <Content title={(data as any).toString()} key={parentKey}>
        {isParentArray ? `${(data as any).toString()}` : `${parentKey}: ${(data as any).toString()}`}
      </Content>
    );
  };

  return (
    <div
      style={{
        display: 'flex',
        flexDirection: 'column',
        gap: 20,
        alignItems: 'center',
      }}
    >
      <ControlButton onClick={toggleAllRow}>{expanded.root ? 'Collapse all' : 'Expand all'}</ControlButton>
      {renderRows(jsonData, Array.isArray(jsonData), 'root', 'root')}
    </div>
  );
}

export default NestedTable;

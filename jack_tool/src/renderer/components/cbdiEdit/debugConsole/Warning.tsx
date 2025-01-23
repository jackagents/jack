/* eslint-disable jsx-a11y/no-static-element-interactions */
/* eslint-disable jsx-a11y/click-events-have-key-events */
import { IModelWarning } from 'misc/types/cbdiEdit/cbdiEditModel';
import { WarningOutlined, ArrowDropDownOutlined, ArrowDropUpOutlined } from '@mui/icons-material';
import { useState } from 'react';

function Warning({ warning }: { warning: IModelWarning }) {
  const [isExpanded, setIsExpanded] = useState(false);

  return (
    <div>
      <div
        style={{
          display: 'flex',
          padding: '10px 0 0 20px',
          fontSize: 16,
          justifyContent: 'space-between',
        }}
      >
        <div style={{ display: 'flex', gap: 10, alignItems: 'center' }}>
          <div>
            <WarningOutlined style={{ color: 'yellow', fontSize: 20 }} />
          </div>
          <div style={{ display: 'flex', flexDirection: 'column' }}>
            <div>{warning.title}</div>
          </div>
        </div>
        <div style={{ display: 'flex', gap: 10, alignItems: 'center' }}>
          <button type="button" style={{ padding: '2px 10px', fontSize: 14, border: '1px solid black' }} onClick={warning.fixHandler}>
            Fix
          </button>
          <div
            hidden={!warning.detail}
            title={isExpanded ? 'Collapse' : 'Expand'}
            style={{ cursor: 'pointer' }}
            onClick={() => setIsExpanded(!isExpanded)}
          >
            {isExpanded ? <ArrowDropDownOutlined style={{ fontSize: 26 }} /> : <ArrowDropUpOutlined style={{ fontSize: 26 }} />}
          </div>
        </div>
      </div>
      {isExpanded && <div style={{ fontSize: 14, paddingLeft: 50 }}>{warning.detail}</div>}
    </div>
  );
}

export default Warning;

import React from 'react';
import { styled, IconButton } from '@mui/material';
import ArrowDropUpIcon from '@mui/icons-material/ArrowDropUp';
import ArrowDropDownIcon from '@mui/icons-material/ArrowDropDown';
import { formatMicrosecondOffset } from 'misc/utils/common/rendererUtils';
import agentImgSvg from 'assets/cbdi/icons/agent_icon.svg';
import { BDILog, Event } from 'types/cbdi/cbdi-types-non-flatbuffer';
import { getBdiLogTitle } from './Helper';
import CbdiMessageContent from './CbdiMessageContent';

interface Props {
  message: Event;
  id: string;
  handleClickLog: (id: string) => void;
  active: boolean;
}

const MessageContainer = styled('li')({
  border: '1px solid black',
});

const TitleContainer = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  paddingLeft: 10,
  backgroundColor: '#dddddd',
  '&:hover': {
    backgroundColor: '#555555',
  },
  '&.checked': {
    backgroundColor: '#068cfa96',
  },
});

const DetailContainer = styled('div')({
  padding: '0 15px',
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'space-evenly',
  backgroundColor: 'white',
  alignItems: 'flex-start',
  fontSize: 14,
  gap: 5,
  '& *:nth-of-type(2)': {
    alignSelf: 'flex-end',
  },
});

const AgentIconContainer = styled('div')({
  display: 'flex',
  alignItems: 'center',
  gap: 5,
});

function IntentionMessageView({ message, active, id, handleClickLog }: Props) {
  const bdiLog = message as BDILog;
  const { sender, timestampUs } = bdiLog;
  const scroller = document.getElementById('scroller-log-view');

  return (
    <MessageContainer>
      <TitleContainer
        onClick={() => {
          handleClickLog(id);

          if (scroller && scroller.scrollTop <= 0) {
            scroller.scrollTo(0, 1);
          }
        }}
        className={active ? 'checked' : ''}
      >
        <div
          style={{
            display: 'flex',
            flexDirection: 'column',
            fontSize: 12,
            fontWeight: 700,
            width: '50%',
            wordBreak: 'break-word',
          }}
        >
          <div>{getBdiLogTitle(bdiLog)}</div>
          <div>{bdiLog.logType}</div>
        </div>
        <div
          style={{
            display: 'flex',
            alignItems: 'center',
            fontSize: 12,
            fontWeight: 400,
            flexGrow: 0,
          }}
        >
          <div>{formatMicrosecondOffset(timestampUs)}</div>
          <IconButton
            title={active ? 'close detail view' : 'open detail view'}
            sx={{ p: '5px' }}
            onClick={(_e) => {
              handleClickLog(id);
            }}
          >
            {active ? <ArrowDropDownIcon /> : <ArrowDropUpIcon />}
          </IconButton>
        </div>
      </TitleContainer>
      <DetailContainer hidden={!active}>
        <CbdiMessageContent bdiLog={bdiLog} />
        <AgentIconContainer>
          <div>{`${sender.name} Agent`}</div>
          <img alt="" width={24} src={agentImgSvg} />
        </AgentIconContainer>
        <div>{formatMicrosecondOffset(timestampUs)}</div>
      </DetailContainer>
    </MessageContainer>
  );
}

export default React.memo(IntentionMessageView);

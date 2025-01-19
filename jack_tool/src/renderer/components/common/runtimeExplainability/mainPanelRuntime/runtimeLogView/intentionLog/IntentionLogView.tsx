import React from 'react';
import { styled } from '@mui/material';
import ViewportList from 'react-viewport-list';
import { Event } from 'types/cbdi/cbdi-types-non-flatbuffer';
import IntentionMessageView from './children/IntentionMessageView';

const MessageContainer = styled('ul')({
  display: 'flex',
  flexDirection: 'column',
  backgroundColor: '#ffffff',
  height: '100%',
});

interface Props {
  agentIntentionLogArr: Event[];
}

function IntentionLogView({ agentIntentionLogArr }: Props) {
  const ref = React.useRef<HTMLUListElement | null>(null);

  const [listActive, setListActive] = React.useState(Object.fromEntries(agentIntentionLogArr.map((x) => [x.eventId, false])));

  const handleClickLog = React.useCallback(
    (id: string) => {
      setListActive({ ...listActive, [id]: !listActive[id] });
    },
    [listActive],
  );

  return (
    <MessageContainer id="scroller-log-view" ref={ref}>
      <ViewportList items={agentIntentionLogArr} overscan={200} viewportRef={ref}>
        {(item) => (
          <IntentionMessageView
            key={item.eventId}
            id={item.eventId}
            message={item}
            handleClickLog={handleClickLog}
            active={listActive[item.eventId]}
          />
        )}
      </ViewportList>
    </MessageContainer>
  );
}

export default React.memo(IntentionLogView);

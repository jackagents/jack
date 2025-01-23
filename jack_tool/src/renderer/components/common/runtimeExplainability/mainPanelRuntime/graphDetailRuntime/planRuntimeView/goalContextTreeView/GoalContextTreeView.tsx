import ExpandableContainer from 'components/common/expandableContainer/ExpandableContainer';
import NestedTable from 'components/common/nestedTable/NestedTable';
import { MessageData } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';
import React from 'react';

interface Props {
  goalContextMsg: MessageData[] | undefined;
}

export default function GoalContextTreeView({ goalContextMsg }: Props) {
  /* ------------------------------ useMemo hooks ----------------------------- */

  // remove empty goal context message
  // remove name and field
  const processedGoalContextMsg = React.useMemo(() => {
    if (goalContextMsg !== undefined) {
      return goalContextMsg
        .map((el) => {
          if (el && Object.keys(el).length !== 0) {
            return el;
          }
          return undefined;
        })
        .filter((el) => el !== undefined) as Record<string, any>[];
    }
    return [];
  }, [goalContextMsg]);

  return (
    <ExpandableContainer
      customStyles={{
        top: 120,
        left: 0,
        width: 500,
        height: 300,
        gap: 10,
        padding: 10,
      }}
      buttonTitle="Goal Context Tree"
    >
      <div
        style={{
          overflow: 'auto',
          gap: 10,
          display: 'flex',
          flexDirection: 'column',
        }}
      >
        <div>Goal Message</div>
        {processedGoalContextMsg.length === 0 ? 'No goal message available' : <NestedTable jsonData={processedGoalContextMsg} />}
      </div>
    </ExpandableContainer>
  );
}

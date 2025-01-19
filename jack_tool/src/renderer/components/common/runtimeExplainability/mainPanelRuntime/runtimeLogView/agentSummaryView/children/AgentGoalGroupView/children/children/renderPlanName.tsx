/* eslint-disable jsx-a11y/click-events-have-key-events */
/* eslint-disable jsx-a11y/no-static-element-interactions */
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import { RenderCellProps } from 'react-data-grid';

interface Props {
  props: RenderCellProps<any>;
  handleClickRow: (row: Record<string, any>) => void;
}

export function renderPlanName({ props, handleClickRow }: Props) {
  return (
    <div
      onClick={(e) => {
        e.preventDefault();
        e.stopPropagation();
        handleClickRow(props.row);
      }}
      style={{ overflow: 'hidden', textOverflow: 'ellipsis' }}
      title={removeBeforeFirstDotAndDot(props.row.planName)}
    >
      {removeBeforeFirstDotAndDot(props.row.planName)}
    </div>
  );
}

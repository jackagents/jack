import { useCallback, useMemo, useState } from 'react';
import DataGrid, { CellClickArgs, CellMouseEvent, RenderCellProps } from 'react-data-grid';
import { PlanGroupDic } from '../type';
import './AgentGoalGroupTable.css';
import { renderPlanName } from './children/renderPlanName';

const NodeLengthUnit = 50;
// additional 2px for data grid table border
export const AgentGoalGroupTableWidth = NodeLengthUnit * 8 + 2;
export const AgentGoalGroupTableHeight = NodeLengthUnit * 4;

interface Props {
  planGroupDic: PlanGroupDic;
  selectedPlanTemplateName: string | undefined;
  handleClickRow: (row: Record<string, any>) => void;
}

function AgentGoalGroupTable({ planGroupDic, selectedPlanTemplateName, handleClickRow }: Props) {
  const getInitialColumns = () => {
    if (Object.keys(planGroupDic).length > 3) {
      return [
        {
          key: 'planName',
          name: 'Intention',
          resizable: true,
          width: NodeLengthUnit * 4.5,
          renderCell: (props: RenderCellProps<any>) => renderPlanName({ props, handleClickRow }),
        },
        {
          key: 'active',
          name: 'In Progress',
          resizable: false,
          width: NodeLengthUnit * 1.6,
        },
        { key: 'complete', name: 'Complete', resizable: false, width: NodeLengthUnit * 1.6 },
      ];
    }
    return [
      {
        key: 'planName',
        name: 'Intention',
        resizable: true,
        width: NodeLengthUnit * 4.8,
        renderCell: (props: RenderCellProps<any>) => renderPlanName({ props, handleClickRow }),
      },
      {
        key: 'active',
        name: 'In Progress',
        resizable: false,
        width: NodeLengthUnit * 1.6,
      },
      { key: 'complete', name: 'Complete', resizable: false, width: NodeLengthUnit * 1.6 },
    ];
  };

  /* --------------------------------- useState hooks --------------------------------- */
  const [adjustedColumns, setAdjustedColumns] = useState(getInitialColumns());

  const rows = useMemo(() => {
    const mrows: Record<string, any>[] = [];
    Object.entries(planGroupDic).forEach(([planTemplateName, planGoalInfoItems]) => {
      mrows.push({
        planName: planTemplateName,
        active: planGoalInfoItems.active.length,
        complete: planGoalInfoItems.complete.length,
      });
    });
    return mrows;
  }, [planGroupDic]);

  /* ----------------------------- callbacks ----------------------------- */
  const handleClick = (args: CellClickArgs<Record<string, any>, unknown>, event: CellMouseEvent) => {
    event.stopPropagation();
    event.preventDefault();
    event.preventGridDefault();
    handleClickRow(args.row);
  };

  const handleColumnResize = useCallback(
    (index: number, width: number) => {
      if (index !== 0) {
        return;
      }
      let totalWidth = NodeLengthUnit * 8;
      if (rows.length > 3) {
        totalWidth = NodeLengthUnit * 7.7;
      }
      setAdjustedColumns((prev) => {
        const result = [...prev];
        result[index].width = width;
        const restWidth = totalWidth - width;
        result[1].width = restWidth - prev[2].width;

        return result;
      });
    },
    [rows.length],
  );

  return (
    <DataGrid
      defaultColumnOptions={{
        sortable: true,
        resizable: true,
      }}
      columns={adjustedColumns}
      rows={rows}
      onCellClick={handleClick}
      className="agentGoalGroupTable"
      onColumnResize={handleColumnResize}
      rowClass={(row) => {
        if (row.planName === selectedPlanTemplateName) {
          return 'row-selected';
        }
        return undefined;
      }}
    />
  );
}

export default AgentGoalGroupTable;

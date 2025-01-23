import 'react-data-grid/lib/styles.css';
import DataGrid, { CellClickArgs, CellMouseEvent, ColumnOrColumnGroup, SortColumn } from 'react-data-grid';
import './AgentGoalPursueTable.css';
import { useCallback, useMemo, useState } from 'react';
import { useExplainabilityContext } from 'components/common/runtimeExplainability/context/explainabilityContext';
import { capitalize } from 'misc/utils/cbdiEdit/Helpers';
import { removeBeforeFirstDotAndDot } from 'misc/utils/common/commonUtils';
import { AgentGoalGroupDic, getGoalContextMsgArr } from '../../helper';
/* --------------------------------- Styles --------------------------------- */

interface Props {
  selectedGoalTemplateName: string | undefined;
  selectedPlanTemplateName: string | undefined;
  agentGoalGroupDic: AgentGoalGroupDic;
}

function EmptyRowsRenderer() {
  return <div style={{ textAlign: 'center', gridColumn: '1/-1' }}>Nothing to show </div>;
}

function AgentGoalPursueTable({ selectedGoalTemplateName, selectedPlanTemplateName, agentGoalGroupDic }: Props) {
  /* ---------------------------- useContext hooks ---------------------------- */
  const { setInspectAgentGoal, inspectNodeData, inspectAgentGoal } = useExplainabilityContext();

  // get initial columns and rows
  const [columns, rows] = useMemo(() => {
    const mtableDataColumns = [
      {
        key: 'goalId',
        name: 'Goal ID',
        resizable: true,
        sortable: true,
        draggable: true,
        hidden: false,
      },
      {
        key: 'planTemplateName',
        name: 'Plan',
        resizable: true,
        sortable: true,
        draggable: true,
        hidden: false,
      },
      {
        key: 'messageTemplateName',
        name: 'Message',
        resizable: true,
        sortable: true,
        draggable: true,
        hidden: false,
      },
      {
        key: 'status',
        name: 'Status',
        resizable: true,
        sortable: true,
        draggable: true,
        hidden: false,
      },
      {
        key: 'intentionResult',
        name: 'Intention Result',
        resizable: true,
        sortable: true,
        draggable: true,
        hidden: false,
      },
      {
        key: 'goalResult',
        name: 'Goal Result',
        resizable: true,
        sortable: true,
        draggable: true,
        hidden: false,
      },
      {
        key: 'bdiLogLevel',
        name: 'Bdi Log Level',
        resizable: true,
        sortable: true,
        draggable: true,
        hidden: false,
      },
    ];

    const mrows: Record<string, any>[] = [];
    if (selectedGoalTemplateName) {
      const agentGoalGroup = agentGoalGroupDic[selectedGoalTemplateName];
      if (agentGoalGroup) {
        agentGoalGroup.active.forEach((goalInfoItem) => {
          const isGoalInTable = (() => {
            if (!selectedPlanTemplateName) {
              return true;
            }
            if (selectedPlanTemplateName === goalInfoItem.planTemplateName) {
              return true;
            }
            if (selectedPlanTemplateName === 'No Plan' && goalInfoItem.planTemplateName === undefined) {
              return true;
            }
            return false;
          })();
          if (isGoalInTable) {
            const newRow: Record<string, any> = {
              goalId: goalInfoItem.goalId,
              status: 'Active',
              planTemplateName: removeBeforeFirstDotAndDot(goalInfoItem.planTemplateName) || 'No Plan',
              intentionResult: goalInfoItem.intentionResult,
              goalResult: goalInfoItem.goalResult,
              bdiLogLevel: goalInfoItem.bdiLogLevel,
            };
            const { goalContextMsg } = goalInfoItem;
            if (goalContextMsg) {
              const goalContextMsgArr = getGoalContextMsgArr(goalContextMsg);

              goalContextMsgArr.forEach((el) => {
                const key = Object.keys(el)[0];
                const value = el[key];
                if (!mtableDataColumns.some((col) => col.key === key)) {
                  mtableDataColumns.push({ key, name: capitalize(key), resizable: true, sortable: true, draggable: true, hidden: false });
                }
                newRow[key] = value;
              });
            }
            mrows.push(newRow);
          }
        });

        agentGoalGroup.complete.forEach((goalInfoItem) => {
          const isGoalInTable = (() => {
            if (!selectedPlanTemplateName) {
              return true;
            }
            if (selectedPlanTemplateName === goalInfoItem.planTemplateName) {
              return true;
            }
            if (selectedPlanTemplateName === 'No Plan' && goalInfoItem.planTemplateName === undefined) {
              return true;
            }
            return false;
          })();
          if (isGoalInTable) {
            const newRow: Record<string, any> = {
              goalId: goalInfoItem.goalId,
              status: 'Complete',
              planTemplateName: removeBeforeFirstDotAndDot(goalInfoItem.planTemplateName) || 'No Plan',
              intentionResult: goalInfoItem.intentionResult,
              goalResult: goalInfoItem.goalResult,
              bdiLogLevel: goalInfoItem.bdiLogLevel,
            };
            const { goalContextMsg } = goalInfoItem;
            if (goalContextMsg) {
              const goalContextMsgArr = getGoalContextMsgArr(goalContextMsg);
              goalContextMsgArr.forEach((el) => {
                const key = Object.keys(el)[0];
                const value = el[key];
                if (!mtableDataColumns.some((col) => col.key === key)) {
                  mtableDataColumns.push({ key, name: capitalize(key), resizable: true, sortable: true, draggable: true, hidden: false });
                }
                newRow[key] = value;
              });
            }
            mrows.push(newRow);
          }
        });
      }
    }

    return [mtableDataColumns, mrows];
  }, [agentGoalGroupDic, selectedGoalTemplateName, selectedPlanTemplateName]);

  /* ----------------------------- useState hooks ----------------------------- */
  const [sortColumns, setSortColumns] = useState<readonly SortColumn[]>([]);
  // const [columnsOrder, setColumnsOrder] = useState((): readonly number[] => columns.map((_, index) => index));
  const [processedColumns, setProcessedColumns] = useState<any[]>(columns);
  const [draggingColumnSourceKey, setDraggingColumnSourceKey] = useState<string | undefined>();
  const [draggingColumnTargetKey, setDraggingColumnTargetKey] = useState<string | undefined>();
  /* ----------------------------- useCallback hooks ----------------------------- */
  const onSortColumnsChange = useCallback((sortingColumns: SortColumn[]) => {
    setSortColumns(sortingColumns.slice(-1));
  }, []);

  const onColumnsReorder = useCallback((sourceKey: string, targetKey: string) => {
    setProcessedColumns((prev) => {
      const sourceColumnOrderIndex = prev.findIndex((column) => column.key === sourceKey);
      const targetColumnOrderIndex = prev.findIndex((column) => column.key === targetKey);
      const sourceColumnOrder = prev[sourceColumnOrderIndex];

      const newColumnsOrder = prev.slice();
      newColumnsOrder.splice(sourceColumnOrderIndex, 1);
      newColumnsOrder.splice(targetColumnOrderIndex, 0, sourceColumnOrder);
      return newColumnsOrder;
    });
  }, []);

  const onColFilterItemClick = useCallback((column: any) => {
    setProcessedColumns((prev) =>
      prev.map((col) => {
        if (col.key === column.key) {
          col.hidden = !col.hidden;
          return col;
        }
        return col;
      }),
    );
  }, []);

  /* ----------------------------- useMemo hooks ----------------------------- */

  const sortedRows = useMemo((): readonly Record<string, any>[] => {
    if (sortColumns.length === 0) return rows;
    const { columnKey, direction } = sortColumns[0];

    let msortedRows: Record<string, any>[] = [...rows];

    msortedRows = msortedRows.sort((a, b) => JSON.stringify(a[columnKey]).localeCompare(JSON.stringify(b[columnKey])));
    return direction === 'DESC' ? msortedRows.reverse() : msortedRows;
  }, [rows, sortColumns]);

  /* ----------------------------- callbacks ----------------------------- */
  const handleClick = (args: CellClickArgs<Record<string, any>, unknown>, event: CellMouseEvent) => {
    event.preventDefault();
    event.stopPropagation();
    event.preventGridDefault();
    if (inspectAgentGoal?.goalId !== args.row.goalId && inspectNodeData && inspectNodeData.agentBusAddress) {
      setInspectAgentGoal({ agentId: inspectNodeData.agentBusAddress.id, goalId: args.row.goalId });
    } else {
      setInspectAgentGoal(undefined);
    }
  };

  return (
    <div style={{ height: '100%', display: 'flex', flexDirection: 'column' }}>
      <div style={{ flex: '0 0 auto', display: 'flex', gap: 5, padding: 5, overflowX: 'auto' }}>
        {processedColumns.map((column) => {
          const buttonBackgroundColor = (() => {
            if (draggingColumnTargetKey === column.key && draggingColumnSourceKey !== column.key) {
              return 'lightblue';
            }
            if (column.hidden) {
              return undefined;
            }
            return 'grey';
          })();

          return (
            <button
              type="button"
              onClick={() => onColFilterItemClick(column)}
              key={column.key}
              draggable
              onDragStart={() => {
                setDraggingColumnSourceKey(column.key);
              }}
              onDragEnd={() => {
                if (draggingColumnSourceKey && draggingColumnTargetKey && draggingColumnSourceKey !== draggingColumnTargetKey) {
                  onColumnsReorder(draggingColumnSourceKey, draggingColumnTargetKey);
                }
                setDraggingColumnSourceKey(undefined);
                setDraggingColumnTargetKey(undefined);
              }}
              onDragEnter={() => {
                setDraggingColumnTargetKey(column.key);
              }}
              style={{
                backgroundColor: buttonBackgroundColor,
                fontSize: 16,
                padding: 5,
                whiteSpace: 'nowrap',
              }}
            >
              {column.name}
            </button>
          );
        })}
      </div>
      <div style={{ flex: 1, overflow: 'auto' }}>
        <DataGrid
          defaultColumnOptions={{
            sortable: true,
            resizable: true,
          }}
          onCellClick={handleClick}
          columns={processedColumns.filter((col) => !col.hidden)}
          rows={sortedRows}
          sortColumns={sortColumns}
          onSortColumnsChange={onSortColumnsChange}
          onColumnsReorder={onColumnsReorder}
          className="agentGoalPursueTable"
          renderers={{ noRowsFallback: <EmptyRowsRenderer /> }}
          rowClass={(row) => {
            if (row.goalId === inspectAgentGoal?.goalId) {
              return 'row-selected';
            }
            return undefined;
          }}
        />
      </div>
    </div>
  );
}

export default AgentGoalPursueTable;

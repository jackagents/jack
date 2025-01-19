/* eslint-disable react/jsx-props-no-spreading */
/* eslint-disable @typescript-eslint/no-unused-vars */
import { List, SelectChangeEvent, styled } from '@mui/material';
import NumericInput from 'react-numeric-input';
import DraggableList from 'react-draggable-list';
import React from 'react';
import { ThemedMenuItem, ThemedSelect } from 'components/cbdiEdit/themedComponents/ThemedComponents';
import { CBDIEditorProject, CBDIEditorProjectTactic } from 'misc/types/cbdiEdit/cbdiEditModel';
import { ModuleConcept, PlanOrderType, CBDIEditorModuleConceptWithId } from 'misc/types/cbdiEdit/cbdiEditTypes';
import BooleanValueToggler from 'components/cbdiEdit/BooleanValueToggler.tsx/BooleanValueToggler';
import ModuleCocneptSelector from 'components/cbdiEdit/detail/children/ModuleCocneptSelector/ModuleCocneptSelector';
import PlanListItem, { PlanListCommonProps } from './PlanListItem';

const MAX_PLAN_LOOP_VALUE = 4294967295;
/* --------------------------------- Styles --------------------------------- */
const Root = styled('div')({
  display: 'flex',
  flexDirection: 'column',
  fontSize: '.9em',
  gap: 5,
});

const RowContainer = styled('div')({
  display: 'flex',
  justifyContent: 'space-between',
  alignItems: 'center',
  margin: 5,
  gap: 5,
  overflow: 'hidden',
});

const ButtonsContainer = styled('div')({
  display: 'flex',
  justifyContent: 'center',
  padding: 15,
  gap: 10,
});

const StyledButton = styled('button')({
  display: 'inline-block',
  padding: '8px 10px',
  fontSize: 14,
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
  '&:active': {
    backgroundColor: '#0d47a1',
    boxShadow: 'none',
  },
  '&:disabled': {
    opacity: 0.5,
    cursor: 'not-allowed',
    backgroundColor: '#bdbdbd',
    '&:hover': {
      backgroundColor: '#bdbdbd',
    },
    '&:active': {
      backgroundColor: '#bdbdbd',
      boxShadow: 'none',
    },
  },
});

const PlanListContainer = styled(List)({
  paddingBottom: 5,
  width: 'calc(100%-20px)',
  margin: 5,
  padding: 10,
  overflowX: 'hidden',
});

const GrayMask = styled('div')({
  position: 'absolute',
  top: 0,
  left: 0,
  width: '100%',
  height: '100%',
  background: 'rgba(221, 221, 221, 0.5)',
  pointerEvents: 'none',
  zIndex: 999,
});

const TextView = styled('div')({
  fontFamily: 'Roboto, Helvetica, Arial, sans-serif',
  paddingLeft: 5,
});

/* -------------------------------- Constants ------------------------------- */

interface Props {
  immutable: boolean;
  project: CBDIEditorProject;
  tactic: CBDIEditorProjectTactic;
  allPlanList: ModuleConcept[];
  textColor: string;
  onToggleUserPlanList: () => void;
  onTogglePlanOrder: (planOrder: PlanOrderType) => void;
  onChangePlanLoop: (planLoop: number) => void;
  onAddPlanItem: (planModuleConcept: ModuleConcept) => void;
  onRemovePlanItem: (deletingId: string) => void;
  onReorderPlanList: (newPlanList: CBDIEditorModuleConceptWithId[]) => void;
  onDefaultPlanPolicy: () => void;
}

export default function PlanPolicy({
  immutable,
  project,
  tactic,
  allPlanList,
  textColor,
  onToggleUserPlanList,
  onTogglePlanOrder,
  onChangePlanLoop,
  onAddPlanItem,
  onRemovePlanItem,
  onReorderPlanList,
  onDefaultPlanPolicy,
}: Props) {
  /* ------------------------------ useRef hooks ------------------------------ */
  const planListContainer = React.useRef(null);

  /* -------------------------------- Callbacks ------------------------------- */
  const handlePlanLoopInputChange = (value: number | null, _stringValue: string, _input: HTMLInputElement) => {
    if (value !== null) {
      onChangePlanLoop(value);
    }
  };

  const handleFormat = (value: number | null) => {
    if (value !== null) {
      const parsedValue = parseInt(value as unknown as string, 10);
      if (parsedValue === MAX_PLAN_LOOP_VALUE) {
        return 'Infinite';
      }
    }
    return value as unknown as string;
  };

  const handleParse = (stringValue: string) => {
    if (stringValue.includes('Infinite')) {
      return MAX_PLAN_LOOP_VALUE;
    }
    if (stringValue.trim() === '') {
      return 0;
    }
    const parsedValue = parseInt(stringValue, 10);
    if (Number.isNaN(parsedValue)) {
      return null;
    }
    return parseInt(stringValue, 10);
  };

  const handleKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    const { key } = event;
    const isNumeric = /^\d$/.test(key);
    const isBackspace = key === 'Backspace';
    const isArrowKey = /^Arrow/.test(key);
    if (!isNumeric && !isBackspace && !isArrowKey) {
      event.preventDefault();
    }
    if (isBackspace && tactic.plan_loop === MAX_PLAN_LOOP_VALUE) {
      event.preventDefault();
      onChangePlanLoop(0);
    }
  };
  /* ------------------------------- Components ------------------------------- */

  return (
    <Root>
      {immutable ? <TextView>(Read only)</TextView> : null}
      {/* Use Plan List */}
      <RowContainer title="If this is set true, then the plan list is respected in the tactic, if it is set false then all plans will be considered">
        <BooleanValueToggler
          label="Use Plan List"
          currentValue={tactic.use_plan_list}
          onToggle={() => {
            onToggleUserPlanList();
          }}
          disabled={immutable}
        />
      </RowContainer>
      {/* Plan List */}
      <RowContainer
        title={`This means that when you execute the goal, it will try plan a first, if\nthat fails then plan b e.t.c and then c. Note we also have "plan b"\nrepeated at the end, this is allowed and means that after c, it has to\ntry b.\nThis list can also be empty, which means the plans that are chosen for\nthe goal is up to c-bdi, and so nothing needs to be done on the model\nside.`}
      >
        Plan List
      </RowContainer>
      <PlanListContainer
        ref={planListContainer}
        sx={{
          overflow: planListContainer ? 'auto' : '',
          maxHeight: planListContainer ? '200px' : '',
          border: (theme) => `1px solid ${theme.editor.detailView.textColor}`,
          pointerEvents: tactic.use_plan_list && !immutable ? 'auto' : 'none',
        }}
      >
        <DraggableList<CBDIEditorModuleConceptWithId, PlanListCommonProps, PlanListItem>
          list={tactic.plan_list}
          itemKey={(item) => item.id}
          template={PlanListItem}
          onMoveEnd={(newList) => {
            if (immutable) {
              return;
            }
            onReorderPlanList(newList as CBDIEditorModuleConceptWithId[]);
          }}
          container={() => (planListContainer ? planListContainer.current! : document.body)}
          commonProps={{ project, onRemovePlanItem }}
        />
        <ModuleCocneptSelector
          moduleConceptOptions={allPlanList}
          immutable={immutable}
          isSelectingItem={false}
          emptyOptionLabel="Add a new plan"
          onChange={onAddPlanItem}
          hasborder="true"
        />
        {tactic.use_plan_list && !immutable ? null : <GrayMask />}
      </PlanListContainer>

      {/* Plan Order */}
      <RowContainer>
        <div style={{ whiteSpace: 'nowrap' }}>Plan Order</div>
        <ThemedSelect
          hasborder="true"
          value={tactic.plan_order}
          onChange={(event: SelectChangeEvent<unknown>) => {
            onTogglePlanOrder(event.target.value as PlanOrderType);
          }}
        >
          {Object.values(PlanOrderType).map((planOrder: PlanOrderType, index: number) => {
            const displayText = (() => {
              if (planOrder === 'ExcludePlanAfterAttempt') {
                return 'Exclude Plan After Attempt';
              }
              if (planOrder === 'ChooseBestPlan') {
                return 'Choose Best Plan';
              }
              return planOrder;
            })();
            return (
              <ThemedMenuItem key={index as number} value={planOrder}>
                <span>{displayText}</span>
              </ThemedMenuItem>
            );
          })}
        </ThemedSelect>
      </RowContainer>

      {/* Plan Loop */}
      <RowContainer>
        <div style={{ whiteSpace: 'nowrap' }}>Plan Loop</div>
        <div
          style={{
            display: 'flex',
            width: 220,
            justifyContent: 'space-between',
            alignItems: 'center',
          }}
        >
          <NumericInput
            style={{
              input: {
                width: 130,
                height: 32,
              },
            }}
            type="number"
            precision={0}
            value={tactic.plan_loop}
            onChange={handlePlanLoopInputChange}
            step={1}
            min={0}
            max={MAX_PLAN_LOOP_VALUE}
            format={handleFormat}
            parse={handleParse}
            onKeyDown={handleKeyDown}
          />
          <StyledButton
            onClick={() => {
              onChangePlanLoop(MAX_PLAN_LOOP_VALUE);
            }}
          >
            Infinite
          </StyledButton>
        </div>
      </RowContainer>
      <ButtonsContainer>
        <StyledButton
          title="Reset to default policy"
          disabled={immutable}
          onClick={() => {
            onDefaultPlanPolicy();
          }}
        >
          Reset Policy
        </StyledButton>
      </ButtonsContainer>
    </Root>
  );
}

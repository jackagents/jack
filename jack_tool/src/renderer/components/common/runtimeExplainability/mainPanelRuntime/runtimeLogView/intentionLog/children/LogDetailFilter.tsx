/* eslint-disable react/no-unused-prop-types */
import { styled, Divider } from '@mui/material';
import Select, { ActionMeta, MultiValue } from 'react-select';
import {
  SeverityFilterData,
  BdiLogTypeFilterData,
} from 'misc/types/cbdi/cbdi-models-non-flatbuffer';
// import { BDILogLevel } from 'misc/types/cbdi/cbdiTypes';
import { BDILogLevel } from 'misc/types/cbdi/cbdi-types-non-flatbuffer';

const severityOptions: SeverityFilterData[] = [
  { value: BDILogLevel.NORMAL, label: 'Normal' },
  { value: BDILogLevel.IMPORTANT, label: 'Important' },
  { value: BDILogLevel.CRITICAL, label: 'Critical' },
];

const bdiLogTypeOptions: BdiLogTypeFilterData[] = [
  { value: 'goal', label: 'Goal' },
  { value: 'intention', label: 'Intention' },
  { value: 'action', label: 'Action' },
  { value: 'sleep', label: 'Sleep' },
  { value: 'condition', label: 'Condition' },
];

const FilterContiner = styled('div')({
  display: 'flex',
  alignItems: 'center',
});

const SelectorContainer = styled('div')({
  margin: 10,
  flex: 1,
});

const DetailFilterContainer = styled('div')({
  flexGrow: 1,
  display: 'flex',
  flexDirection: 'column',
  padding: 10,
});

interface Props {
  isShowingDetailFilter: boolean;
  fontSizeScale: number;
  severityFilteredOptions: SeverityFilterData[];
  handleSeverityFilterItemChange: (
    newValue: MultiValue<SeverityFilterData>,
    _actionMeta: ActionMeta<any>
  ) => void;
  cbdiFilteredOptions: BdiLogTypeFilterData[];
  handleCbdiFilterItemChange: (
    newValue: MultiValue<BdiLogTypeFilterData>,
    _actionMeta: ActionMeta<any>
  ) => void;
  timeValues: number[];
  handleTimeRangeChange: (
    _event: Event,
    newValue: number | number[],
    activeThumb: number
  ) => void;
}

export default function LogDetailFilter(props: Props) {
  return (
    <DetailFilterContainer hidden={!props.isShowingDetailFilter}>
      <FilterContiner>
        <div>Severity:&nbsp;&nbsp;&nbsp;</div>
        <SelectorContainer>
          <Select
            defaultValue={[]}
            isMulti
            options={severityOptions}
            onChange={props.handleSeverityFilterItemChange}
            value={props.severityFilteredOptions}
          />
        </SelectorContainer>
      </FilterContiner>
      <Divider />
      <FilterContiner>
        <div>Keywords:</div>
        <SelectorContainer>
          <Select
            defaultValue={[]}
            isMulti
            options={bdiLogTypeOptions}
            onChange={props.handleCbdiFilterItemChange}
            value={props.cbdiFilteredOptions}
          />
        </SelectorContainer>
      </FilterContiner>
      <Divider />
      {/* comment out time range slider for now */}
      {/* <SliderContainer>
        <div>Time range:</div>
        <TimeValueContainer>
          {props.timeValues &&
            `${convertNumToTime(props.timeValues[0])} to ${convertNumToTime(
              props.timeValues[1]
            )}`}
        </TimeValueContainer>
        <Slider
          size="small"
          valueLabelDisplay="auto"
          valueLabelFormat={convertNumToTime}
          value={props.timeValues}
          onChange={props.handleTimeRangeChange}
          disableSwap
          max={1440}
        />
      </SliderContainer> */}
    </DetailFilterContainer>
  );
}

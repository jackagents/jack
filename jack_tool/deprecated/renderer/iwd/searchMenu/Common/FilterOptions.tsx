import { Grid, FormControlLabel, styled, Stack } from '@mui/material';
import React from 'react';
import { PoIType, FilterCheckBox } from 'types/iwd/iwdTypes';
import { GreyCheckBox } from 'components/common/base/BaseContainer';

const initCheckBoxState = {
  hostile: true,
  friendly: true,
  unknown: true,
};

const FilterBox = styled(Stack)({
  // margin: '0 0 0 10px',
  width: '100%',
  height: '60%',
});

const CustomGrid = styled(Grid)({
  marginLeft: '10px',
  height: '30%',
});

const CustomFormControlLabel = styled(FormControlLabel)(({ theme }) => ({
  color: theme.custom?.text.color,
  '& .MuiTypography-root': {
    fontSize: theme.custom?.text.size,
  },
}));

interface Props {
  onFilter: (checkBoxes: FilterCheckBox) => void;
}

function FilterOptions(props: Props) {
  const [checkBoxState, setCheckBoxState] = React.useState<FilterCheckBox>({
    ...initCheckBoxState,
  });

  React.useEffect(() => {
    if (props.onFilter) props.onFilter(checkBoxState);
  }, [checkBoxState]);

  const handleChange = React.useCallback(
    (_event: React.ChangeEvent<HTMLInputElement>, value: PoIType) => {
      switch (value) {
        case PoIType.UNKNOWN:
          setCheckBoxState({
            ...checkBoxState,
            unknown: !checkBoxState.unknown,
          });
          break;
        case PoIType.FRIENDLY:
          setCheckBoxState({
            ...checkBoxState,
            friendly: !checkBoxState.friendly,
          });
          break;
        case PoIType.HOSTILE:
          setCheckBoxState({
            ...checkBoxState,
            hostile: !checkBoxState.hostile,
          });
          break;
        default:
          break;
      }
    },
    [checkBoxState]
  );

  const filterByType = () => {
    const types = Object.values(PoIType);

    return types.map((value, index) => {
      return (
        <CustomGrid key={value}>
          <CustomFormControlLabel
            label={value}
            control={
              <GreyCheckBox
                onChange={(e) => {
                  handleChange(e, value);
                }}
                checked={checkBoxState[value]}
              />
            }
          />
        </CustomGrid>
      );
    });
  };

  return <FilterBox>{filterByType()}</FilterBox>;
}

export default React.memo(FilterOptions);

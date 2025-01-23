import { Button, MenuItem, Stack, styled, Tooltip } from '@mui/material';
import React from 'react';
import TextRotationAngledownIcon from '@mui/icons-material/TextRotationAngledown';
import TextRotationAngleupIcon from '@mui/icons-material/TextRotationAngleup';
import { SelectTextField } from 'components/common/base/BaseContainer';

const SortStack = styled(Stack)({
  padding: '5px',
  width: '100%',
  height: '30%',
  justifyContent: 'center',
  alignItems: 'center',
});

const SortIconBtn = styled(Button)(({ theme }) => ({
  color: theme.custom?.text.color,
  backgroundColor: theme.custom?.sortIcon.bgColor,
  maxWidth: '2.5vw',
  maxHeight: '2.5vw',
  minWidth: '2.5vw',
  minHeight: '2.5vw',
}));

const SortIconAsc = styled(TextRotationAngleupIcon)({
  width: '1.5vw',
  height: '1.5vw',
});

const SortIconDes = styled(TextRotationAngledownIcon)({
  width: '1.5vw',
  height: '1.5vw',
});

interface Option {
  value: string;
  label: string;
}

interface Props {
  options: Option[];
  default: string;
  onSort: (sort: string, asc: boolean) => void;
}

function SortOptions(props: Props) {
  const [sort, setSort] = React.useState(props.default);
  const [ascOrder, setAscOrder] = React.useState(true);

  const handleClick = React.useCallback(() => {
    setAscOrder(!ascOrder);
  }, [ascOrder]);

  const handleChange = React.useCallback(
    (event: React.ChangeEvent<HTMLInputElement>) => {
      setSort(event.target.value);
    },
    []
  );

  React.useEffect(() => {
    props.onSort(sort, ascOrder);
  }, [sort, ascOrder]);

  return (
    <SortStack direction="row">
      <SelectTextField
        select
        label="Sort by"
        value={sort}
        onChange={handleChange}
      >
        {props.options.map((option) => (
          <MenuItem key={option.value} value={option.value}>
            {option.label}
          </MenuItem>
        ))}
      </SelectTextField>

      <Tooltip title={(ascOrder && 'Ascending') || 'Descending'}>
        <SortIconBtn onClick={handleClick}>
          {(ascOrder && <SortIconAsc />) || <SortIconDes />}
        </SortIconBtn>
      </Tooltip>
    </SortStack>
  );
}

export default React.memo(SortOptions);

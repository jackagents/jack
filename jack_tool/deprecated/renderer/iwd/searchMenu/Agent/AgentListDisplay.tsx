import { Stack, styled } from '@mui/material';
import React from 'react';
import { useSelector } from 'react-redux';
import { CONSTANT_STRING } from 'constant/common/cmConstants';
import { FilterCheckBox, TabIndex } from 'types/iwd/iwdTypes';
import {
  ContentBox,
  StyledSearchBar,
  StyledStack,
} from 'components/common/base/BaseContainer';
import FunctionLabel from 'components/common/functionLabel/FunctionLabel';
import { RootState } from 'projectRedux/Store';
import FilterOptions from '../Common/FilterOptions';
import ItemList from '../Common/ItemList';
import SortOptions from '../Common/SortOptions';

enum SortOption {
  name = 'name',
  type = 'type',
  status = 'status',
  dateCreated = 'dateCreated',
}

const sortOptions = [
  {
    value: SortOption.name,
    label: CONSTANT_STRING.SORT_LABEL.NAME,
  },
  {
    value: SortOption.type,
    label: CONSTANT_STRING.SORT_LABEL.TYPE,
  },
  {
    value: SortOption.status,
    label: CONSTANT_STRING.SORT_LABEL.STATUS,
  },
  {
    value: SortOption.dateCreated,
    label: CONSTANT_STRING.SORT_LABEL.DATE_CREATED,
  },
];

function AgentListDisplay() {
  const { agents } = useSelector((state: RootState) => state.iwd);

  const data = React.useMemo(() => {
    return agents;
  }, [agents]);

  const searchText = React.useRef<string>('');

  const [filteredData, setFilteredData] = React.useState(data);

  const search = React.useCallback(
    (text: string) => {
      if (!data || data.length <= 0) {
        return;
      }

      searchText.current = text;

      const filtered = data.filter((entity) => {
        return entity.address.name.toLowerCase().includes(text);
      });

      setFilteredData([...filtered]);
    },
    [data]
  );

  React.useEffect(() => {
    // Update filteredData with latest data
    // Apply search filter
    search(searchText.current);
  }, [data]);

  const onSort = React.useCallback((sortBy: string, isAscending: boolean) => {},
  []);

  const onFilter = React.useCallback(
    (checkBoxes: FilterCheckBox) => {},
    [data]
  );

  return (
    <StyledStack direction="column">
      <StyledSearchBar
        className="styled-search-bar"
        onSearch={(text: string) => {
          search(text);
        }}
      />

      <ContentBox sx={{ height: '20%' }}>
        <FunctionLabel text={CONSTANT_STRING.FILTER} />
        {/* <SortOptions
          options={sortOptions}
          default={SortOption.dateCreated}
          onSort={onSort}
        />
        <FilterOptions onFilter={onFilter} /> */}
      </ContentBox>

      <ContentBox sx={{ height: '65%' }}>
        <FunctionLabel text={CONSTANT_STRING.AGENTS} />
        <ItemList tab={TabIndex.agent} agents={filteredData} />
      </ContentBox>
    </StyledStack>
  );
}
export default React.memo(AgentListDisplay);

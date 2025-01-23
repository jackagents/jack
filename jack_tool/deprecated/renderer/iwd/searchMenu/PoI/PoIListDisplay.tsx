import React from 'react';
import { useSelector } from 'react-redux';
import { CONSTANT_STRING } from 'constant/common/cmConstants';
import {
  FilterCheckBox,
  PointOfInterestType,
  PoIType,
  TabIndex,
} from 'types/iwd/iwdTypes';
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
    value: SortOption.dateCreated,
    label: CONSTANT_STRING.SORT_LABEL.DATE_CREATED,
  },
];

const defaultSort = {
  sortBy: SortOption.dateCreated,
  isAscending: true,
};

const defaultFilter: FilterCheckBox = {
  hostile: true,
  friendly: true,
  unknown: true,
};

function PoIListDisplay() {
  const { pointsOfInterest } = useSelector((state: RootState) => state.iwd);

  const data = React.useMemo(() => {
    return pointsOfInterest;
  }, [pointsOfInterest]);

  const sortOption = React.useRef<typeof defaultSort>(defaultSort);
  const searchText = React.useRef<string>('');
  const filterOption = React.useRef<FilterCheckBox>(defaultFilter);

  const [filteredData, setFilteredData] = React.useState(data);

  const sort = (sortData: PointOfInterestType[]) => {
    const sortArr = [...sortData];

    sortArr.sort((a, b) => {
      let textA = '';
      let textB = '';

      switch (sortOption.current.sortBy) {
        case SortOption.name:
          textA = a.name || a.id;
          textB = b.name || b.id;
          break;
        case SortOption.type:
          textA = a.type;
          textB = b.type;
          break;
        case SortOption.dateCreated:
          textA = a.dateCreated;
          textB = b.dateCreated;
          break;
        default:
          break;
      }

      if (sortOption.current.isAscending) {
        return textA.localeCompare(textB, undefined, {
          numeric: true,
          sensitivity: 'base',
        });
      }

      return textB.localeCompare(textA, undefined, {
        numeric: true,
        sensitivity: 'base',
      });
    });

    return sortArr;
  };

  const checkBoxFilter = () => {
    let result: PointOfInterestType[] = [];

    if (filterOption.current.unknown) {
      const filtered = data.filter((x) => x.type === PoIType.UNKNOWN);
      result = [...result, ...filtered];
    }

    if (filterOption.current.friendly) {
      const filtered = data.filter((x) => x.type === PoIType.FRIENDLY);

      result = [...result, ...filtered];
    }

    if (filterOption.current.hostile) {
      const filtered = data.filter((x) => x.type === PoIType.HOSTILE);

      result = [...result, ...filtered];
    }

    return result;
  };

  const applyFilterSearchSort = React.useCallback(
    (text: string) => {
      if (!data) {
        return;
      }

      searchText.current = text;

      const filtered = checkBoxFilter().filter((ele) => {
        return (
          ele.name?.toLowerCase().includes(text) ||
          ele.id.toLowerCase().includes(text)
        );
      });

      setFilteredData([...sort(filtered)]);
    },
    [data]
  );

  const onSort = React.useCallback(
    (sortBy: string, isAscending: boolean) => {
      sortOption.current = {
        sortBy: sortBy as SortOption,
        isAscending,
      };

      setFilteredData([...sort(filteredData)]);
    },
    [filteredData]
  );

  const onFilter = React.useCallback(
    (checkBoxes: FilterCheckBox) => {
      filterOption.current = checkBoxes;
      applyFilterSearchSort(searchText.current);
    },
    [data]
  );

  React.useEffect(() => {
    // Apply search and sort filter
    applyFilterSearchSort(searchText.current);
  }, [data]);

  return (
    <StyledStack direction="column">
      <StyledSearchBar
        className="styled-search-bar"
        onSearch={(text) => {
          applyFilterSearchSort(text);
        }}
      />

      <ContentBox sx={{ height: '20%' }}>
        <FunctionLabel text={CONSTANT_STRING.FILTER} />
        <SortOptions
          options={sortOptions}
          default={SortOption.dateCreated}
          onSort={onSort}
        />
        <FilterOptions onFilter={onFilter} />
      </ContentBox>

      <ContentBox sx={{ height: '65%' }}>
        <FunctionLabel text={CONSTANT_STRING.POINTS_OF_INTEREST} />
        <ItemList tab={TabIndex.poi} pois={filteredData} />
      </ContentBox>
    </StyledStack>
  );
}

export default React.memo(PoIListDisplay);

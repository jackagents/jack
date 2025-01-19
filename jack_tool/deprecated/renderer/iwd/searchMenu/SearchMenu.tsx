import { Stack, styled } from '@mui/material';
import React from 'react';
import { GridContainer } from 'components/common/base/BaseContainer';
import { TabIndex } from 'types/iwd/iwdTypes';
import SearchDisplayTabs from './SearchDisplayTabs';
import PoIListDisplay from './PoI/PoIListDisplay';
import AgentListDisplay from './Agent/AgentListDisplay';

const SearchDisplayTabStack = styled(Stack)({
  width: '100%',
  height: '100%',
});

function SearchMenu() {
  const [tab, setTab] = React.useState<TabIndex>(TabIndex.agent);

  /**
   * Callback on tab change
   */
  const handleTabChange = React.useCallback((value: string) => {
    setTab(value as TabIndex);
  }, []);

  /**
   * Get display of the tab
   * @returns Agent display or PointOfInerest display
   */
  const getListDisplay = () => {
    switch (tab) {
      case TabIndex.agent: {
        return <AgentListDisplay />;
      }
      case TabIndex.poi: {
        return <PoIListDisplay />;
      }
      default: {
        return null;
      }
    }
  };

  return (
    <GridContainer className="search-menu" container>
      <SearchDisplayTabStack direction="column">
        <SearchDisplayTabs onTabChange={handleTabChange} />
        {getListDisplay()}
      </SearchDisplayTabStack>
    </GridContainer>
  );
}

export default React.memo(SearchMenu);

import React from 'react';
import { useDispatch, useSelector } from 'react-redux';
import {
  Agent,
  CurrentAgent,
  CurrentSelectElement,
  PointOfInterestType,
  SelectableElement,
  TabIndex,
} from 'types/iwd/iwdTypes';
import { iwdActions } from 'projectRedux/reducers/iwd/iwdClientReducer';
import { RootState } from 'projectRedux/Store';
import {
  CustomItemList,
  Item,
  ItemButton,
  ItemText,
} from 'components/common/base/BaseContainer';

interface Props {
  tab: TabIndex;
  pois?: PointOfInterestType[];
  agents?: Agent[];
}

function ItemList(props: Props) {
  const dispatch = useDispatch();

  const { currentSelectElement } = useSelector((state: RootState) => state.iwd);

  const [currentClickedItem, setCurrentClickedItem] =
    React.useState<number>(-1);

  const data = React.useMemo(() => {
    if (props.tab === TabIndex.agent) {
      return props.agents;
    }
    if (props.tab === TabIndex.poi) {
      return props.pois;
    }

    return [];
  }, [props]);

  React.useEffect(() => {
    if (!currentSelectElement || !data) return;

    // Default select none
    let index = -1;

    // Select agent
    if (
      currentSelectElement.type === SelectableElement.AGENT &&
      props.tab === TabIndex.agent
    ) {
      const currAgent = currentSelectElement.value as CurrentAgent;

      index = (data as Agent[]).findIndex((x) => x.address.id === currAgent.id);
    }
    // Select poi
    else if (
      currentSelectElement.type === SelectableElement.POI &&
      props.tab === TabIndex.poi
    ) {
      const currPoI = currentSelectElement.value as PointOfInterestType;

      index = (data as PointOfInterestType[]).findIndex(
        (x) => x.id === currPoI.id
      );
    }

    setCurrentClickedItem(index);
  }, [currentSelectElement, data]);

  const handleClick = React.useCallback(
    (element: Agent | PointOfInterestType, index: number) => {
      // Open the config menu
      let payload: CurrentSelectElement = null;

      switch (props.tab) {
        case TabIndex.agent: {
          payload = {
            type: SelectableElement.AGENT,
            value: { id: (element as Agent).address.id },
          };
          break;
        }
        case TabIndex.poi: {
          payload = {
            type: SelectableElement.POI,
            value: { ...(element as PointOfInterestType) },
          };
          break;
        }
        default:
          break;
      }

      if (!payload) {
        console.log('SearchMenu handleClick payload is null');
        return;
      }

      dispatch(iwdActions.changeCurrentSelectElement(JSON.stringify(payload)));
      setCurrentClickedItem(index);
    },
    []
  );

  const listItems = React.useMemo(() => {
    if (!data) return [];

    return data.map((item, index) => {
      return (
        <Item
          disablePadding
          key={
            props.tab === 'poi'
              ? (item as PointOfInterestType).id
              : (item as Agent).address.id
          }
        >
          <ItemButton
            sx={
              currentClickedItem === index
                ? (theme) => ({
                    backgroundColor: theme.custom?.item.clicked.bgColor,
                    '&:hover': {
                      backgroundColor:
                        theme.custom?.item.clicked.hoveredBgColor,
                    },
                  })
                : (theme) => ({
                    '&:hover': {
                      backgroundColor: theme.custom?.item.normal.hoveredBgColor,
                    },
                  })
            }
            onClick={() => {
              handleClick(item, index);
            }}
          >
            <ItemText>
              {props.tab === 'poi'
                ? (item as PointOfInterestType).name ||
                  (item as PointOfInterestType).id
                : (item as Agent).address.name}
            </ItemText>
          </ItemButton>
        </Item>
      );
    });
  }, [data, currentClickedItem]);

  return <CustomItemList>{listItems}</CustomItemList>;
}

ItemList.defaultProps = {
  pois: [],
  agents: [],
};

export default React.memo(ItemList);

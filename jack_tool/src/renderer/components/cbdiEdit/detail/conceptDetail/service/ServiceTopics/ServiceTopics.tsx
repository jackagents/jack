import ModuleCocneptSelector from 'components/cbdiEdit/detail/children/ModuleCocneptSelector/ModuleCocneptSelector';
import { CBDIEditorRootConceptType, CBDIEditorServiceTopicSchema, ModuleConcept } from 'misc/types/cbdiEdit/cbdiEditTypes';
import DraggableList from 'react-draggable-list';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import React, { useMemo } from 'react';
import { RootState } from 'projectRedux/Store';
import { getAllObjOptionsForSingleItem, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { CBDIEditorProjectService } from 'misc/types/cbdiEdit/cbdiEditModel';
import { v4 } from 'uuid';
import { FlexCol } from 'components/common/base/BaseContainer';
import ServiceTopicItem, { ServiceTopicCommonProps } from './ServiceTopicItem';

function ServiceTopics({ serviceModuleConcept }: { serviceModuleConcept: ModuleConcept }) {
  /* ---------------------------------- Redux --------------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const dispatch = useDispatch();
  const { updateObjects } = cbdiEditActions;

  /* ------------------------------ useRef hooks ------------------------------ */
  const topicListContainer = React.useRef(null);

  /* ------------------------------ useMemo hooks ----------------------------- */
  const serviceObj = useMemo(
    () => getObjectByModuleConcept(current, serviceModuleConcept)! as CBDIEditorProjectService,
    [current, serviceModuleConcept],
  );

  const allMessageOptions = useMemo(() => getAllObjOptionsForSingleItem(CBDIEditorRootConceptType.MessageConceptType, current), [current]);

  /* -------------------------------- Callbacks ------------------------------- */
  const onReorderTopics = (newTopicList: CBDIEditorServiceTopicSchema[]) => {
    if (serviceObj) {
      const newTacticObj: CBDIEditorProjectService = {
        ...serviceObj,
        topics: newTopicList,
      };
      dispatch(updateObjects(newTacticObj));
    }
  };

  const onAddTopicItem = (topicModuleConcept: ModuleConcept) => {
    if (serviceObj) {
      const topicListItem = {
        name: `newTopic${v4().slice(0, 4)}`,
        message: topicModuleConcept,
      };
      const newServiceObj: CBDIEditorProjectService = {
        ...serviceObj,
        topics: [...serviceObj.topics, topicListItem],
      };
      dispatch(updateObjects(newServiceObj));
    }
  };

  const onRemoveTopicItem = (deletingTopicName: string) => {
    if (serviceObj) {
      const newTopicList = serviceObj.topics.filter((item) => item.name !== deletingTopicName);
      const newServiceObj: CBDIEditorProjectService = {
        ...serviceObj,
        topics: newTopicList,
      };
      dispatch(updateObjects(newServiceObj));
    }
  };

  const onTopicNameChange = (oldTopic: CBDIEditorServiceTopicSchema, name: string) => {
    if (serviceObj) {
      const topics = serviceObj.topics.map((topic) => {
        if (topic.name === oldTopic.name) {
          return { ...topic, name };
        }
        return topic;
      });
      const newServiceObj = { ...serviceObj, topics };
      dispatch(updateObjects(newServiceObj));
    }
  };

  const onTopicMessageChange = (oldTopic: CBDIEditorServiceTopicSchema, message: ModuleConcept) => {
    if (serviceObj) {
      const topics = serviceObj.topics.map((topic) => {
        if (topic.name === oldTopic.name) {
          return { ...topic, message };
        }
        return topic;
      });
      const newServiceObj = { ...serviceObj, topics };
      dispatch(updateObjects(newServiceObj));
    }
  };

  if (!serviceObj) {
    return null;
  }
  return (
    <FlexCol
      ref={topicListContainer}
      sx={{
        overflow: topicListContainer ? 'auto' : '',
        padding: 2,
        gap: 2,
      }}
    >
      <ModuleCocneptSelector
        moduleConceptOptions={allMessageOptions}
        immutable={false}
        isSelectingItem={false}
        emptyOptionLabel="Add a new topic with message"
        onChange={onAddTopicItem}
        hasborder="true"
      />
      <DraggableList<CBDIEditorServiceTopicSchema, ServiceTopicCommonProps, ServiceTopicItem>
        list={serviceObj.topics}
        itemKey={(item) => item.name}
        template={ServiceTopicItem}
        onMoveEnd={(newList) => {
          onReorderTopics(newList as CBDIEditorServiceTopicSchema[]);
        }}
        container={() => (topicListContainer ? topicListContainer.current! : document.body)}
        commonProps={{
          current: current!,
          allTopics: serviceObj.topics,
          onRemoveTopicItem,
          onTopicNameChange,
          onTopicMessageChange,
        }}
      />
    </FlexCol>
  );
}

export default ServiceTopics;

/* eslint-disable react/jsx-props-no-spreading */
import { styled } from '@mui/material';
import Modal from 'react-modal';
import { cbdiEditActions } from 'projectRedux/reducers/cbdiEdit/cbdiEditCurrentClientReducer/cbdiEditCurrentClientReducer';
import { useDispatch, useSelector } from 'react-redux';
import React from 'react';
import './UniversalSearch.css';
import { nodeColor, nodeIcon } from 'misc/icons/cbidEdit/cbdiEditIcon';
import Select from 'react-select';
import { createNewConcept, createNewPlanWithGoal, createNewTacticWithGoal, getObjectByModuleConcept } from 'misc/utils/cbdiEdit/Helpers';
import { v4 } from 'uuid';
import ReactLoading from 'react-loading';
import CloseIcon from '@mui/icons-material/Close';
import fuzzysort from 'fuzzysort';
import { MODE_PROJECT } from 'misc/constant/cbdiEdit/cbdiEditConstant';
import { RootState } from 'projectRedux/Store';
import { editorRendererToRendererEvents } from 'misc/events/cbdiEdit/editEvents';
import { Edge } from 'reactflow';
import { PlanEdgeData, OptionData, Mod, CBDIEditorRootConceptType, CBDIEditorPlanNodeType } from 'misc/types/cbdiEdit/cbdiEditTypes';
import { CBDIEditorProjectPlan, CBDIEditorProjectTactic } from 'misc/types/cbdiEdit/cbdiEditModel';
import { filterCreateOption, getAddingTask, getDefaultOptions, getOptions } from './helper';
import ImageWithBackground from './ImageWithBackground';

function SearchResult({ highlightedText }: { highlightedText: string }) {
  return <div dangerouslySetInnerHTML={{ __html: highlightedText }} />;
}
/* --------------------------------- Styles --------------------------------- */

// modal style
const customStyles: ReactModal.Styles = {
  overlay: {
    zIndex: 9999, // set a high value for the overlay
    position: 'fixed',
    top: 0,
    left: 0,
    right: 0,
    bottom: 0,
    backgroundColor: 'rgba(0, 0, 0, 0.75)',
  },
  content: {
    zIndex: 10000, // set an even higher value for the content
    top: '5%',
    left: '50%',
    right: 'auto',
    bottom: 'auto',
    marginRight: '-50%',
    transform: 'translate(-50%, 0%)',
    padding: 0,
    borderRadius: 6,
  },
};

const Root = styled('div')({
  display: 'flex',
  width: 800,
  minHeight: 580,
});

const SearchRoot = styled('div')({
  width: 480,
  backgroundColor: '#222222',
  display: 'flex',
  justifyContent: 'center',
  paddingTop: 20,
});

const LoadingRoot = styled('div')({
  position: 'absolute',
  width: '90%',
  height: '90%',
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  zIndex: 9,
});

const DetailRoot = styled('div')({
  flexGrow: 1,
  backgroundColor: '#1a1a1a',
});

const CloseButton = styled('div')({
  position: 'absolute',
  top: 10,
  right: 10,
  '&:hover': {
    cursor: 'pointer',
  },
});

const SelectListItemButton = styled('button')({
  marginLeft: 'auto',
  backgroundColor: '#ffffff',
  border: 'none',
  fontSize: 14,
  padding: '4px 8px',
  borderRadius: '4px',
  cursor: 'pointer',
  ':hover': {
    backgroundColor: '#cccccc',
  },
});

// Make sure to bind modal to root element
Modal.setAppElement('#root');

interface Props {
  mode: 'conceptSwitching' | 'taskAdding';
  addingTaskEdge: Edge<PlanEdgeData> | undefined;
  isModalOpen: boolean;
  onCloseModal: () => void;
}

export default function UniversalSearch(props: Props) {
  /* ----------------------------- Redux ----------------------------- */
  const current = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.project.current);
  const mode = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.explorer.mode);
  const selectedTreeNodeConcept = useSelector((state: RootState) => state.cbdiEdit.cbdiEditCurrent.present.selectedTreeNodeConcept);
  const dispatch = useDispatch();
  const { setSelectedTreeNodeConcept, setGraphSelectedNode, putObjects, setExplorerMode, setScrollToSelectedTreeNodeFlag } = cbdiEditActions;
  /* ----------------------------- useState hooks ----------------------------- */
  const [selectedOption, setSelectedOption] = React.useState(null);
  const [inputValue, setInputValue] = React.useState('');
  const [isLoading, setIsloading] = React.useState(false);

  /* ------------------------------ useRef hooks ------------------------------ */
  const selectRef = React.useRef(null);
  /* ------------------------------ useMemo hooks ----------------------------- */
  const allOptions = React.useMemo(() => {
    if (current) {
      return getOptions(props.mode, current);
    }
    return [];
  }, [current, props.mode, selectedTreeNodeConcept]);

  const options = React.useMemo(() => {
    let moptions: OptionData[] = [];
    if (inputValue.trim().length > 0) {
      const fuzzyResults = fuzzysort.go(inputValue, allOptions, {
        key: 'label',
      });

      moptions = fuzzyResults.map((result) => {
        const highlightedLabel = fuzzysort.highlight(result, '<span class="universal-select_fuzzy-highlight">', '</span>');
        return { ...result.obj, label: highlightedLabel };
      }) as OptionData[];
    } else {
      moptions = allOptions;
    }

    const defaultOptions = getDefaultOptions(props.mode, current, selectedTreeNodeConcept, inputValue);

    moptions = [...moptions, ...defaultOptions];

    moptions = moptions.filter((el) => filterCreateOption(props.mode, allOptions, el, inputValue));

    return moptions;
  }, [allOptions, inputValue]);
  /* ----------------------------- useEffect hooks ---------------------------- */

  React.useEffect(() => {
    setSelectedOption(null);
    setInputValue('');
  }, [props.isModalOpen]);

  /* -------------------------------- Callbacks ------------------------------- */
  const handleAfterOpen = () => {
    if (selectRef.current) {
      (selectRef.current as any).focus();
    }
  };

  // add (create concept) task to plan
  const handleAddTask = (data: OptionData) => {
    const { objectType, moduleConcept, value } = data;
    let addingTaskModuleConcept = moduleConcept;
    if (value === `create-${objectType}`) {
      setIsloading(true);
      const id = v4();
      // if it is action/goal
      // create new concept
      if (objectType === CBDIEditorPlanNodeType.ActionPlanNodeType || objectType === CBDIEditorPlanNodeType.GoalPlanNodeType) {
        const createRootType =
          objectType === CBDIEditorPlanNodeType.ActionPlanNodeType
            ? CBDIEditorRootConceptType.ActionConceptType
            : CBDIEditorRootConceptType.GoalConceptType;

        const createdConcepts = createNewConcept(id, createRootType, selectedTreeNodeConcept!.module, inputValue.trim());
        if (createdConcepts.length > 0) {
          addingTaskModuleConcept = createdConcepts[0];
          dispatch(putObjects(createdConcepts));
        }
      }
      setTimeout(() => {
        setIsloading(false);
        const task = getAddingTask(current!, objectType as CBDIEditorPlanNodeType, inputValue, addingTaskModuleConcept);

        if (task && window.mainWindowId) {
          if (props.addingTaskEdge) {
            window.ipcRenderer.sendTo(window.mainWindowId, editorRendererToRendererEvents.editorAddingPlanTaskWithEdge, task, props.addingTaskEdge);
          } else {
            window.ipcRenderer.sendTo(window.mainWindowId, editorRendererToRendererEvents.editorAddingPlanTask, task);
          }
        }
        props.onCloseModal();
      }, 1000);
    } else {
      const task = getAddingTask(current!, objectType as CBDIEditorPlanNodeType, inputValue, moduleConcept);

      if (task && window.mainWindowId) {
        if (props.addingTaskEdge) {
          window.ipcRenderer.sendTo(window.mainWindowId, editorRendererToRendererEvents.editorAddingPlanTaskWithEdge, task, props.addingTaskEdge);
        } else {
          window.ipcRenderer.sendTo(window.mainWindowId, editorRendererToRendererEvents.editorAddingPlanTask, task);
        }
      }
      props.onCloseModal();
    }
  };

  // switch (create concept) concept
  const handleConceptSwitch = (data: OptionData) => {
    const { objectType, moduleConcept, value } = data;

    if (value === `create-${objectType}`) {
      // create new concept
      const index = inputValue.indexOf('>');
      if (index > -1) {
        const id = v4();
        const conceptName = inputValue.substring(index + 1).trim();
        if (conceptName.length > 0) {
          setIsloading(true);
          const createdConcepts = (() => {
            const selectedTreeNode = getObjectByModuleConcept(current, selectedTreeNodeConcept);
            if (
              selectedTreeNodeConcept &&
              selectedTreeNode?._mod !== Mod.Deletion &&
              (selectedTreeNode?._objectType === CBDIEditorRootConceptType.GoalConceptType ||
                selectedTreeNode?._objectType === CBDIEditorRootConceptType.PlanConceptType ||
                selectedTreeNode?._objectType === CBDIEditorRootConceptType.TacticConceptType)
            ) {
              const goalConcept = (() => {
                if (selectedTreeNode?._objectType === CBDIEditorRootConceptType.GoalConceptType) {
                  return selectedTreeNodeConcept;
                }
                if (selectedTreeNode?._objectType === CBDIEditorRootConceptType.PlanConceptType) {
                  return (selectedTreeNode as CBDIEditorProjectPlan).handles;
                }
                if (selectedTreeNode?._objectType === CBDIEditorRootConceptType.TacticConceptType) {
                  return (selectedTreeNode as CBDIEditorProjectTactic).goal;
                }
                return selectedTreeNodeConcept;
              })();
              if (objectType === CBDIEditorRootConceptType.PlanConceptType) {
                return [createNewPlanWithGoal(id, goalConcept, conceptName)];
              }
              if (objectType === CBDIEditorRootConceptType.TacticConceptType) {
                return [createNewTacticWithGoal(id, goalConcept, conceptName)];
              }
            }
            return createNewConcept(id, objectType as CBDIEditorRootConceptType, selectedTreeNodeConcept!.module, conceptName);
          })();
          if (createdConcepts) {
            dispatch(putObjects(createdConcepts));
          }
          setTimeout(() => {
            setIsloading(false);
            props.onCloseModal();
          }, 1000);
        }
      }
    } else if (moduleConcept) {
      dispatch(setSelectedTreeNodeConcept(moduleConcept));
      dispatch(setGraphSelectedNode(moduleConcept));
      dispatch(setScrollToSelectedTreeNodeFlag());
    }
    props.onCloseModal();
    if (mode !== MODE_PROJECT) {
      dispatch(setExplorerMode(MODE_PROJECT));
    }
  };

  /* ------------------------------ Custom label ------------------------------ */
  const formatConceptSwitchingOptionLabel = (data: OptionData) => {
    const { label, objectType, value } = data;
    const isNew = value === `create-${objectType}`;
    const displayLabel = (() => {
      if (isNew) {
        const index = inputValue.indexOf('>');
        const substring = index > -1 ? inputValue.substring(index + 1).trim() : '';
        return `${label}: "${substring}"`;
      }
      return label;
    })();
    return (
      <div style={{ display: 'flex', alignItems: 'center', gap: 20 }}>
        {!isNew ? (
          <ImageWithBackground src={nodeIcon[objectType]} title={objectType} width={30} height={30} backgroundColor={nodeColor[objectType]} />
        ) : null}
        <SearchResult highlightedText={displayLabel} />
        <SelectListItemButton
          onClick={(e) => {
            e.stopPropagation();
            handleConceptSwitch(data);
          }}
        >
          {isNew ? 'Create' : 'Go to'}
        </SelectListItemButton>
      </div>
    );
  };

  const formatTaskAddingOptionLabel = (data: OptionData) => {
    const { label, objectType, value } = data;

    const isNew = value === `create-action` || value === `create-goal` || value === `create-condition` || value === `create-sleep`;
    const displayLabel = (() => {
      if (isNew) {
        return `${label}: "${inputValue.trim()}"`;
      }

      return label;
    })();

    return (
      <div style={{ display: 'flex', alignItems: 'center', gap: 20 }}>
        {!isNew ? (
          <ImageWithBackground src={nodeIcon[objectType]} title={objectType} width={30} height={30} backgroundColor={nodeColor[objectType]} />
        ) : null}
        <SearchResult highlightedText={displayLabel} />
        <SelectListItemButton
          onClick={(e) => {
            e.stopPropagation();
            handleAddTask(data);
          }}
        >
          {isNew ? 'Create and Add' : 'Add to Plan'}
        </SelectListItemButton>
      </div>
    );
  };

  /* ------------------------------- Components ------------------------------- */
  return (
    <Modal
      closeTimeoutMS={200}
      isOpen={props.isModalOpen}
      onRequestClose={props.onCloseModal}
      style={customStyles}
      contentLabel="Search Modal"
      onAfterOpen={handleAfterOpen}
    >
      <Root>
        <CloseButton onClick={props.onCloseModal}>
          <CloseIcon style={{ color: 'white' }} />
        </CloseButton>
        <SearchRoot>
          {isLoading ? (
            <LoadingRoot>
              <ReactLoading type="spinningBubbles" color="white" width="20%" height="20%" />
            </LoadingRoot>
          ) : null}
          <Select
            ref={selectRef}
            className="universal-select-container"
            classNamePrefix="universal-select"
            options={options}
            filterOption={() => true}
            placeholder={props.mode === 'taskAdding' ? 'Search for task' : 'Search for concept'}
            onInputChange={(newValue, actionMeta) => {
              if (actionMeta.action === 'input-change') {
                setInputValue(newValue);
              }
            }}
            onChange={(newValue) => {
              if (newValue) {
                if (props.mode === 'taskAdding') {
                  handleAddTask(newValue);
                } else {
                  handleConceptSwitch(newValue);
                }
              }
            }}
            value={selectedOption}
            inputValue={inputValue}
            menuIsOpen
            formatOptionLabel={props.mode === 'conceptSwitching' ? formatConceptSwitchingOptionLabel : formatTaskAddingOptionLabel}
            isSearchable
          />
        </SearchRoot>
        <DetailRoot>
          {props.mode === 'conceptSwitching' && (
            <div
              style={{
                display: 'flex',
                flexDirection: 'column',
                gap: 10,
                marginTop: 50,
                color: 'gray',
                fontSize: 18,
              }}
            >
              <div>Create new concept:</div>
              <div>+ [Concept Type] &gt; [Concept Name]</div>
            </div>
          )}
        </DetailRoot>
      </Root>
    </Modal>
  );
}

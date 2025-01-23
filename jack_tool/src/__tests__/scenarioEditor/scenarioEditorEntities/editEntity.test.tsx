import '@testing-library/jest-dom/extend-expect';
import { cleanup, fireEvent, getByText, screen } from '@testing-library/react';
import RightToolBar from 'components/common/scenarioEditor/rightToolBar/RightToolBar';
import { initState, renderWithProviders } from 'root/mocks/testUtils/testUtils';
import { mockEntity, mockNestedEntity } from 'root/mocks/mockEntity/mockEntity';
import { mockComponentTemplatesNested } from 'root/mocks/mockTemplate/mockTemplate';
import { EScenarioEditorTools } from 'misc/enum/scenarioEditor/scenarioEditorEnum';

describe('Edit entity', () => {
  beforeEach(() => {
    cleanup();
  });

  it('edit prop of entity', async () => {
    const { reduxStore } = renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.ENTITIES,
        currentSelectedExistEntity: mockEntity,
      },
    });

    // Expand the component
    fireEvent.click(screen.getByTestId('ChevronRightIcon'));

    const textfield: HTMLInputElement = screen.getByDisplayValue('testinput');
    expect(textfield).toBeInTheDocument();

    const newValue = 'new value';
    fireEvent.change(textfield, { target: { value: newValue } });

    expect(textfield.value).toBe(newValue);

    fireEvent.blur(textfield);

    expect(reduxStore.dispatch).toBeCalledTimes(1);
    expect(reduxStore.dispatch).toBeCalledWith({
      payload: {
        ...mockEntity,
        components: {
          ...mockEntity.components,
          EmptyComponent: {
            ...mockEntity.components.EmptyComponent,
            testProp: {
              ...mockEntity.components.EmptyComponent.testProp,
              value: newValue,
            },
          },
        },
      },
      type: 'scenarioEditorEntities/setEntity',
    });
  });

  it('edit nested entity', async () => {
    const { reduxStore } = renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.ENTITIES,
        currentSelectedExistEntity: mockNestedEntity,
      },
    });

    // Expand the component
    fireEvent.click(screen.getByTestId('ChevronRightIcon'));

    const textfield: HTMLInputElement = screen.getByDisplayValue('123');
    expect(textfield).toBeInTheDocument();

    const newValue = 52;
    fireEvent.change(textfield, { target: { value: newValue } });

    expect(textfield.value).toBe(newValue.toString());

    fireEvent.blur(textfield);

    expect(reduxStore.dispatch).toBeCalledTimes(1);
    expect(reduxStore.dispatch).toBeCalledWith({
      payload: {
        ...mockNestedEntity,
        components: {
          ...mockNestedEntity.components,
          TestCustomComponent1: {
            ...mockNestedEntity.components.TestCustomComponent1,
            nested1: {
              ...mockNestedEntity.components.TestCustomComponent1.nested1,
              value: {
                ...mockNestedEntity.components.TestCustomComponent1.nested1
                  .value,
                nested2: {
                  ...mockNestedEntity.components.TestCustomComponent1.nested1
                    .value.nested2,
                  value: {
                    ...mockNestedEntity.components.TestCustomComponent1.nested1
                      .value.nested2.value,
                    nested3: {
                      ...mockNestedEntity.components.TestCustomComponent1
                        .nested1.value.nested2.value.nested3,
                      value: {
                        ...mockNestedEntity.components.TestCustomComponent1
                          .nested1.value.nested2.value.nested3.value,
                        field: {
                          ...mockNestedEntity.components.TestCustomComponent1
                            .nested1.value.nested2.value.nested3.value.field,
                          value: newValue,
                        },
                      },
                    },
                  },
                },
              },
            },
          },
        },
      },
      type: 'scenarioEditorEntities/setEntity',
    });
  });

  it('edit nested array entity', async () => {
    const { reduxStore } = renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.ENTITIES,
        currentSelectedExistEntity: mockNestedEntity,
      },
    });

    // Expand the component
    fireEvent.click(screen.getByTestId('ChevronRightIcon'));

    const textfield: HTMLInputElement = screen.getByDisplayValue('456');
    expect(textfield).toBeInTheDocument();

    const newValue = 888;
    fireEvent.change(textfield, { target: { value: newValue } });

    expect(textfield.value).toBe(newValue.toString());

    fireEvent.blur(textfield);

    expect(reduxStore.dispatch).toBeCalledTimes(1);
    expect(reduxStore.dispatch).toBeCalledWith({
      payload: {
        ...mockNestedEntity,
        components: {
          ...mockNestedEntity.components,
          TestCustomComponent1: {
            ...mockNestedEntity.components.TestCustomComponent1,
            basic1: {
              ...mockNestedEntity.components.TestCustomComponent1.basic1,
              value: {
                ...mockNestedEntity.components.TestCustomComponent1.basic1
                  .value,
                customArray: {
                  ...mockNestedEntity.components.TestCustomComponent1.basic1
                    .value.customArray,
                  value: [newValue],
                },
              },
            },
          },
        },
      },
      type: 'scenarioEditorEntities/setEntity',
    });
  });

  it('add and remove nested array prop', async () => {
    const { reduxStore } = renderWithProviders(<RightToolBar />, {
      context: {
        ...initState,
        currentTool: EScenarioEditorTools.ENTITIES,
        currentSelectedExistEntity: mockNestedEntity,
        componentsTemplate: mockComponentTemplatesNested,
      },
    });

    // Expand the component
    fireEvent.click(screen.getByTestId('ChevronRightIcon'));

    // Click add element
    fireEvent.click(screen.getByRole('button', { name: 'Add element' }));
    expect(reduxStore.dispatch).toBeCalledTimes(1);
    expect(reduxStore.dispatch).toBeCalledWith({
      payload: {
        ...mockNestedEntity,
        components: {
          ...mockNestedEntity.components,
          TestCustomComponent1: {
            ...mockNestedEntity.components.TestCustomComponent1,
            basic1: {
              ...mockNestedEntity.components.TestCustomComponent1.basic1,
              value: {
                ...mockNestedEntity.components.TestCustomComponent1.basic1
                  .value,
                customArray: {
                  ...mockNestedEntity.components.TestCustomComponent1.basic1
                    .value.customArray,
                  value: [456, 0],
                },
              },
            },
          },
        },
      },
      type: 'scenarioEditorEntities/setEntity',
    });

    const element = screen.getByTestId('array-prop-customArray');
    expect(element).toBeInTheDocument();
  });
});

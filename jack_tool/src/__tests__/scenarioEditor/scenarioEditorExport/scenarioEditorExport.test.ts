import scenarioEditorManager from 'main/scenarioEditorManager/scenarioEditorManager';
import { testSavedData } from '../../../../mocks/exportData/testSavedData';
import { testExportedData } from '../../../../mocks/exportData/testExportedData';

describe('scenarioEditor save file validate', () => {
  it('export scenario to events', () => {
    const data = {
      ...testSavedData,
    };

    const expectedResult = { ...testExportedData };

    const content = scenarioEditorManager.export(
      './export_deliveryrobot.json',
      data,
      () => {}
    );

    expect(JSON.parse(content)).toEqual(expectedResult);
  });
});

import fs from 'fs';
import { FlatEdgeMetaDataModel, FlatMetaData, FlatNodeMetaDataModel } from 'misc/types/cbdiEdit/cbdiEditModel';
import path from 'path';

export const getMetaDataPath = (projectFilePath: string): string => {
  const projectBaseName = path.basename(projectFilePath);

  // Regular expression to match '.mod.jack.json' or '.cbdi'
  const regex = /\.(mod\.cbdi\.json|cbdi)$/;

  // Replace the matched suffix with an empty string
  const projectBaseNameWithoutExtension = projectBaseName.replace(regex, '');

  const metaDataBaseName = `${projectBaseNameWithoutExtension}.jack.meta`;

  const dirname = path.dirname(projectFilePath);
  const metaDataPath = path.join(dirname, metaDataBaseName);
  return metaDataPath;
};

const parseEdgeMetaDataCsvLine = (line: string): any[] => {
  // Find the positions of the first, second, and last commas
  const firstCommaIndex = line.indexOf(',');
  const secondCommaIndex = line.indexOf(',', firstCommaIndex + 1);
  const lastCommaIndex = line.lastIndexOf(',');

  // Extract the fields based on the positions of the commas
  const SourceId = line.substring(0, firstCommaIndex);
  const TargetId = line.substring(firstCommaIndex + 1, secondCommaIndex);
  const ControlPoints = line.substring(secondCommaIndex + 1, lastCommaIndex);
  const Timestamp = line.substring(lastCommaIndex + 1);

  // Return the fields as an array
  return [SourceId, TargetId, ControlPoints, Timestamp];
};

export const readMetaDataFromCsv = (metaDataFilePath: string) => {
  const flatMetaData: FlatMetaData = { node: [], edge: [] };
  // Read the CSV file synchronously
  const csvContent = fs.readFileSync(metaDataFilePath, 'utf8');

  // Split the CSV content into rows
  const csvRows = csvContent.split('\n');
  let currentSection: 'node' | 'edge' | null = null;
  csvRows.forEach((row) => {
    if (row.startsWith('#')) {
      // This is a header line, determine the section
      if (row.includes('Node Data')) {
        currentSection = 'node';
      } else if (row.includes('Edge Data')) {
        currentSection = 'edge';
      }
    } else if (row.endsWith('Timestamp')) {
      // skip header
    } else if (currentSection && row !== '') {
      // This is a data line, parse it
      if (currentSection === 'node') {
        // Assuming the node data format hasn't changed
        const [NodeId, PositionX, PositionY, Timestamp] = row.split(',');
        const positionX = parseFloat(PositionX);
        const positionY = parseFloat(PositionY);
        const timestamp = parseInt(Timestamp, 10);

        const nodeData = {
          NodeId,
          PositionX: positionX,
          PositionY: positionY,
          Timestamp: timestamp,
        };
        flatMetaData.node.push(nodeData as FlatNodeMetaDataModel);
      } else if (currentSection === 'edge') {
        const [SourceId, TargetId, ControlPoints, Timestamp] = parseEdgeMetaDataCsvLine(row);
        const controlPoints = JSON.parse(ControlPoints);

        const timestamp = parseInt(Timestamp, 10);

        const edgeData = {
          SourceId,
          TargetId,
          ControlPoints: controlPoints,
          Timestamp: timestamp,
        };

        flatMetaData.edge.push(edgeData as FlatEdgeMetaDataModel);
      }
    }
  });

  return flatMetaData;
};

// Function to convert an array of objects to a CSV string
const arrayToCsv = (data: any[], headers: string[]): string => {
  // Convert the headers array into a CSV header line
  const csvHeader = headers.join(',');

  // Convert each object in the data array to a CSV row
  const csvRows = data.map((obj) => {
    // For each header, find the corresponding value in the object
    // If the value is an object (e.g., for ControlPoints), stringify it
    const row = headers.map((header) => {
      const value = obj[header];
      if (typeof value === 'object' && value !== null) {
        return JSON.stringify(value);
      }
      return value;
    });
    // Join the row values with commas to form a CSV row
    return row.join(',');
  });

  // Join the header line and the rows with newlines to form the full CSV string
  return `${csvHeader}\n${csvRows.join('\n')}`;
};

export const convertFlatMedaDataToCsv = (flatMedaData: FlatMetaData): string => {
  // Sort nodes by Timestamp
  flatMedaData.node.sort((a, b) => a.Timestamp - b.Timestamp);

  // Sort edges by Timestamp
  flatMedaData.edge.sort((a, b) => a.Timestamp - b.Timestamp);

  // Convert nodes to CSV string
  const nodeCsv = arrayToCsv(flatMedaData.node, ['NodeId', 'PositionX', 'PositionY', 'Timestamp']);

  // Convert edges to CSV string
  const edgeCsv = arrayToCsv(flatMedaData.edge, ['SourceId', 'TargetId', 'ControlPoints', 'Timestamp']);

  // Combine node and edge CSV strings with headers
  return `# Node Data\n${nodeCsv}\n\n# Edge Data\n${edgeCsv}`;
};

// update metaData csv file
export const saveMetaDataCsvFile = (projectFilePath: string, flatMetaDataArr: FlatMetaData | undefined) => {
  if (!flatMetaDataArr) {
    return;
  }
  const metaDataPath = getMetaDataPath(projectFilePath);
  const csvString = convertFlatMedaDataToCsv(flatMetaDataArr);

  // Write CSV content to the file
  fs.writeFileSync(metaDataPath, csvString, 'utf8');

  console.log(`CSV file written to ${metaDataPath}`);
};

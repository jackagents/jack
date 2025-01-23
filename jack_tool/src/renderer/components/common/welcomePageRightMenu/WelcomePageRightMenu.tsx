import NoteAddIcon from '@mui/icons-material/NoteAdd';
import FileOpenIcon from '@mui/icons-material/FileOpen';
import {
  CloseButton,
  Right,
  RightMenuList,
  RightButton,
  RightPathList,
  RecentFileContainer,
  EllipsisText,
} from './style/WelcomePageRightMenuStyledComponents';

interface Props {
  addText?: string;
  openText?: string;
  recentFiles?: string[];
  handleNewProject?: React.MouseEventHandler<HTMLDivElement>;
  handleOpenProject?: React.MouseEventHandler<HTMLDivElement>;
  handleOpenProjectWithPath?: (filePath: string) => void;
  handleDeletePath?: (
    evt: React.MouseEvent<SVGSVGElement, MouseEvent>,
    filePath: string
  ) => void;
}

/**
 * Recent file explorer
 * @param param0
 * @returns
 */
export default function WelcomePageRightMenu({
  addText = 'New Project',
  openText = 'Open Project',
  recentFiles = [],
  handleDeletePath,
  handleNewProject,
  handleOpenProject,
  handleOpenProjectWithPath,
}: Props) {
  return (
    <Right id="welcome-page-right-menu">
      <RightMenuList>
        <span>Start</span>
        <RightButton onClick={handleNewProject}>
          <NoteAddIcon />
          <span>{addText}</span>
        </RightButton>
        <RightButton onClick={handleOpenProject}>
          <FileOpenIcon />
          <span>{openText}</span>
        </RightButton>
      </RightMenuList>
      <span style={{ fontSize: '1.5em' }}>Recent</span>
      <RightPathList>
        {recentFiles.map((filePath, index) => {
          const fileName = filePath.substring(filePath.lastIndexOf('\\') + 1);
          return (
            <RecentFileContainer
              key={index as number}
              onClick={() => {
                if (handleOpenProjectWithPath) {
                  handleOpenProjectWithPath(filePath);
                }
              }}
            >
              <EllipsisText>{fileName}</EllipsisText>
              <EllipsisText title={filePath}>{filePath}</EllipsisText>
              <CloseButton
                onClick={(e) => {
                  if (handleDeletePath) {
                    handleDeletePath(e, filePath);
                  }
                }}
                className="deletePath"
              />
            </RecentFileContainer>
          );
        })}
      </RightPathList>
    </Right>
  );
}

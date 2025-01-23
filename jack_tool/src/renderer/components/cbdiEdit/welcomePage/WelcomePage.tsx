import React from 'react';
import SchoolIcon from '@mui/icons-material/School';
import VideocamIcon from '@mui/icons-material/Videocam';
import ArticleIcon from '@mui/icons-material/Article';
import SystemUpdateAltIcon from '@mui/icons-material/SystemUpdateAlt';
import { styled, useTheme } from '@mui/material';
import { Fluid } from 'components/common/base/BaseContainer';
import blackEditIcon from 'assets/appLogo/editor.png';
import whiteEditIcon from 'assets/appLogo/editor_white.png';
import { request } from 'projectEvents/cbdiEdit/editEvents';
import WelcomePageRightMenu from 'components/common/welcomePageRightMenu/WelcomePageRightMenu';
import packageJson from '../../../../../package.json';

/* --------------------------------- Styles --------------------------------- */
const Root = styled(Fluid)(({ theme }) => ({
  backgroundColor: theme.editor.welcomePage.bgColor,
  color: theme.editor.welcomePage.textColor,
  display: 'flex',
}));

const Left = styled('div')({
  flex: '0 0 60%',
  display: 'flex',
  flexDirection: 'column',
  justifyContent: 'center',
  alignItems: 'center',
});

const MainTitle = styled('div')({
  fontSize: '2.5em',
  fontWeight: 500,
  margin: '15px 0 5px 0',
});

const VersionText = styled('div')({
  fontSize: '0.8em',
  textDecoration: 'underline',
});

const Button = styled('div')(({ theme }) => ({
  padding: 5,
  borderRadius: 5,
  cursor: 'pointer',
  '& .MuiSvgIcon-root': {
    fontSize: '2em',
  },
  '&:hover': {
    backgroundColor: theme.editor.welcomePage.hoveringBgColor,
  },
}));

const LeftButtonGroup = styled('div')({
  display: 'flex',
  justifyContent: 'center',
  alignItems: 'center',
  marginTop: 40,
  gap: 40,
});

const LeftButton = styled(Button)(({ theme }) => ({
  display: 'flex',
  justifyContent: 'center',
  gap: 5,
  alignItems: 'center',
  color: theme.editor.welcomePage.linkColor,
  flexDirection: 'column',
}));

export default function WelcomePage() {
  /* ----------------------------- useTheme hooks ----------------------------- */
  const theme = useTheme();
  /* ----------------------------- useState hooks ----------------------------- */
  const [openRecentPathRerender, setOpenRecentPathRerender] = React.useState(false);
  /* ----------------------------- useEffect hooks ---------------------------- */
  const recentFiles = React.useMemo(() => window.electronStore.get('editorRecentFiles', []) as string[], [openRecentPathRerender]);

  const icon = React.useMemo(() => (theme.theme === 'dark' ? whiteEditIcon : blackEditIcon), [theme.theme]);

  /* -------------------------------- Callbacks ------------------------------- */

  const handleNewProject = () => {
    window.ipcRenderer.invoke(request.project.new);
  };

  const handleOpenProject = () => {
    window.ipcRenderer.invoke(request.project.open);
  };

  const handleOpenProjectWithPath = (filePath: string) => {
    window.ipcRenderer.invoke(request.project.openWithPath, filePath);
    setOpenRecentPathRerender((prevRerender) => !prevRerender);
  };

  const handleDeletePath = (e: React.MouseEvent<SVGSVGElement, MouseEvent>, filePath: string) => {
    e.stopPropagation();
    const newRecentFiles = recentFiles.filter((item) => item !== filePath);
    window.electronStore.set('editorRecentFiles', newRecentFiles);
    setOpenRecentPathRerender((prevRerender) => !prevRerender);
  };

  return (
    <Root>
      <Left>
        <img src={icon} width="50%" alt="" />
        <MainTitle>JACK Editor</MainTitle>
        <VersionText>Version {packageJson.version} </VersionText>
        <LeftButtonGroup>
          <LeftButton>
            <SchoolIcon />
            <span>Tutorial</span>
          </LeftButton>
          <LeftButton>
            <VideocamIcon />
            <span>Videos</span>
          </LeftButton>
          <LeftButton>
            <ArticleIcon />
            <span>Docs</span>
          </LeftButton>
          <LeftButton>
            <SystemUpdateAltIcon />
            <span>Updates</span>
          </LeftButton>
        </LeftButtonGroup>
      </Left>
      <WelcomePageRightMenu
        recentFiles={recentFiles}
        handleDeletePath={handleDeletePath}
        handleNewProject={handleNewProject}
        handleOpenProject={handleOpenProject}
        handleOpenProjectWithPath={handleOpenProjectWithPath}
      />
    </Root>
  );
}

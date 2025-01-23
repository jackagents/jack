const editorIcon = process.platform === 'win32' ? 'assets/appLogo/editor.ico' : 'assets/appLogo/editor.png';

module.exports = function (packageJson) {
  return {
    productName: 'JACK Editor',
    version: packageJson.version,
    icon: editorIcon,
    appId: 'org.erb.EDIT',
  };
};

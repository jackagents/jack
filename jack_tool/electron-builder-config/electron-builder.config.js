/* eslint-disable no-nested-ternary */
const getProductNameVersion = require('./productNameVersionSwitch');
const packageJson = require('../package.json');

const { productName, version, icon, appId } = getProductNameVersion(packageJson);

/* ----------------------------- extraResources ----------------------------- */
const extraResources = ['./assets/**'];

// TODO
// linux icon not working
module.exports = {
  productName: 'JACK Editor',
  artifactName: `${productName}-${version}.${process.platform === 'win32' ? 'exe' : process.platform === 'linux' ? 'AppImage' : 'dmg'}`,
  icon: process.platform === 'win32' ? icon : undefined,
  appId,
  asar: true,
  nsis: {
    perMachine: true,
    oneClick: false,
    allowToChangeInstallationDirectory: true,
    license: 'assets/license/license.md',
    uninstallDisplayName: `${productName} ${version}`,
  },
  asarUnpack: '**\\*.{node,dll}',
  files: ['dist', 'node_modules', 'package.json'],
  afterSign: '.erb/scripts/notarize.js',
  mac: {
    target: {
      target: 'default',
      arch: ['arm64', 'x64'],
    },
    type: 'distribution',
    hardenedRuntime: true,
    entitlements: 'assets/entitlements.mac.plist',
    entitlementsInherit: 'assets/entitlements.mac.plist',
    gatekeeperAssess: false,
  },
  dmg: {
    contents: [
      {
        x: 130,
        y: 220,
      },
      {
        x: 410,
        y: 220,
        type: 'link',
        path: '/Applications',
      },
    ],
  },
  win: {
    target: ['nsis'],
  },
  linux: {
    target: {
      target: 'AppImage',
      arch: ['arm64', 'x64'],
    },
    category: 'Development',
    icon: process.platform === 'linux' ? icon : undefined,
  },
  directories: {
    app: 'release/app',
    buildResources: 'assets',
    output: 'release/build',
  },
  extraResources,
};

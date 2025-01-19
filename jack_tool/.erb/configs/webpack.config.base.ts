/**
 * Base webpack config used across other specific configs
 */

import webpack from 'webpack';
import path from 'path';
import * as tsconfig from '../../tsconfig.json';
import webpackPaths from './webpack.paths';
import { dependencies as externals } from '../../release/app/package.json';

interface TAliases {
  [key: string]: string[];
}
const aliasesPaths: TAliases = tsconfig.compilerOptions.paths;
const configuration: webpack.Configuration = {
  experiments: {
    // Turn this on for bson@5.5.0
    topLevelAwait: true,
  },
  externals: [...Object.keys(externals || {})],
  stats: 'errors-only',

  module: {
    rules: [
      {
        test: /\.[jt]sx?$/,
        exclude: /node_modules/,
        use: {
          loader: 'ts-loader',
          options: {
            transpileOnly: process.env.NODE_ENV === 'production' ? false : true, // if true, it will ignore typecheck
          },
        },
      },
      {
        test: /\.s[ac]ss$/i,
        use: [
          'style-loader',
          'css-loader',
          {
            loader: 'sass-loader',
            options: {
              // Prefer `dart-sass`
              implementation: require('sass'),
            },
          },
        ],
      },
      {
        test: /\.wasm$/,
        // Disables WebPack's opinion where WebAssembly should be,
        // makes it think that it's not WebAssembly
        //
        // Error: WebAssembly module is included in initial chunk.
        type: 'javascript/auto',
        // Tells WebPack that this module should be included as
        // base64-encoded binary file and not as code
        loader: 'base64-loader',
      },
    ],
  },

  output: {
    path: webpackPaths.srcPath,
    // https://github.com/webpack/webpack/issues/1114
    library: {
      type: 'commonjs2',
    },
  },

  /**
   * Determine the array of extensions that should be used to resolve modules.
   */
  resolve: {
    extensions: ['.js', '.jsx', '.json', '.ts', '.tsx'],
    modules: [webpackPaths.srcPath, 'node_modules', '../src'],
    fallback: {
      path: false,
      fs: false,
      Buffer: false,
      process: false,
      crypto: require.resolve('crypto-browserify'),
      os: require.resolve('os-browserify/browser'),
      assert: require.resolve('assert/'),
      url: require.resolve('url/'),
    },
    alias: {
      ...Object.keys(aliasesPaths).reduce((aliases: Record<keyof TAliases, string>, aliasName: string) => {
        const name = aliasName.replace('/*', '');

        if (name === 'assets') {
          aliases[name] = path.resolve(__dirname, '../../assets');
        } else {
          aliases[name.replace('/*', '')] = path.resolve(__dirname, '..', '..', 'src', aliasesPaths[aliasName][0].replace('/*', ''));
        }

        return aliases;
      }, {}),
      'react-d3-tree': path.resolve(__dirname, '..', '..', 'node_modules/react-d3-tree'),
    },
  },

  plugins: [
    new webpack.EnvironmentPlugin({
      NODE_ENV: 'production',
    }),
  ],
};

export default configuration;

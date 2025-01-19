module.exports = {
  extends: ['erb', 'airbnb', 'airbnb-typescript', 'prettier'],
  plugins: ['@typescript-eslint'],
  rules: {
    'import/no-absolute-path': 'off',
    'no-plusplus': 'off',
    'prefer-destructuring': 'off',
    // A temporary hack related to IDE not resolving correct package.json
    'import/no-extraneous-dependencies': 'off',
    'prettier/prettier': ['warn', { endOfLine: 'auto' }],
    'no-empty-pattern': 'off',
    'react/display-name': 'off',
    'react/prop-types': 'off',
    'react/jsx-uses-react': 'off',
    'react/react-in-jsx-scope': 'off',
    'react/destructuring-assignment': 'off',
    'react-hooks/exhaustive-deps': 'warn',
    'react/jsx-boolean-value': 'off',
    'react-hooks/rules-of-hooks': 'error',
    '@typescript-eslint/no-var-requires': 'off',
    '@typescript-eslint/no-empty-function': 'off',
    '@typescript-eslint/ban-types': 'warn',
    '@typescript-eslint/no-explicit-any': 'off',
    '@typescript-eslint/no-unused-vars': 'warn',
    'no-param-reassign': 'warn',
    // 'no-underscore-dangle': ['error', { allowAfterThis: true }],
    'import/prefer-default-export': 'off',
    'react/require-default-props': 'off',
    'no-underscore-dangle': 'off',
    '@typescript-eslint/no-non-null-assertion': 'off',
    'max-len': ['warn', { code: 150 }],
  },
  parserOptions: {
    ecmaVersion: 2020,
    sourceType: 'module',
    project: './tsconfig.json',
    tsconfigRootDir: __dirname,
    createDefaultProgram: true,
  },
  settings: {
    'import/resolver': {
      // See https://github.com/benmosher/eslint-plugin-import/issues/1396#issuecomment-575727774 for line below
      node: {},
      webpack: {
        config: require.resolve('./.erb/configs/webpack.config.eslint.ts'),
      },
    },
    'import/parsers': {
      '@typescript-eslint/parser': ['.ts', '.tsx'],
    },
  },
};
